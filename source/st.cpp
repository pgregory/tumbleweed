/*
    Little Smalltalk, version 3
    Main Driver
    written By Tim Budd, September 1988
    Oregon State University
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "objmemory.h"
#include "names.h"
#include "interp.h"

#ifdef __APPLE__
#	define GLFW_INCLUDE_GLCOREARB
#endif
#include <GLFW/glfw3.h>

#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

ObjectHandle firstProcess;
int initial = 0;    /* not making initial image */

#if defined TW_ENABLE_FFI
extern void initFFISymbols();   /* FFI symbols */
#endif

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

extern "C" void my_nvgFillColor(NVGcontext* vg, NVGcolor color) {
    printf("R: %f G: %f B: %f A: %F\n", color.r, color.g, color.b, color.a);
    nvgFillColor(vg, nvgRGBA(28.0, 30.0, 34.0, 192.0));
}

GLFWwindow* window;
int winWidth, winHeight;
int fbWidth, fbHeight;
float pxRatio;
double mx, my, t, dt;

extern "C" void drawWindow(NVGcontext* vg, const char* title, float x, float y, float w, float h)
{
    nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

	float cornerRadius = 3.0f;
	NVGpaint shadowPaint;
	NVGpaint headerPaint;

	nvgSave(vg);
//	nvgClearState(vg);

	// Window
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x,y, w,h, cornerRadius);
	nvgFillColor(vg, nvgRGBA(28,30,34,192));
//	nvgFillColor(vg, nvgRGBA(0,0,0,128));
	nvgFill(vg);


	nvgRestore(vg);

    nvgEndFrame(vg);

    glfwSwapBuffers(window);
    glfwPollEvents();

}


int main(int argc, char** argv)
{
    FILE *fp;
    char *p, buffer[120];
    NVGcontext* vg = NULL;
    int fontNormal, fontBold;

    strcpy(buffer,"systemImage");
    p = buffer;

    if (argc != 1) p = argv[1];

    fp = fopen(p, "rb");

    if (fp == NULL) 
    {
        sysError("cannot open image", p);
        exit(1);
    }

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwSetErrorCallback(error_callback);

#ifndef _WIN32 // don't require this on win32, and works with more cards
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);

    window = glfwCreateWindow(640, 480, "NanoVG", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, key_callback);

    vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
            printf("Could not init nanovg.\n");
            return -1;
    }

    fontNormal = nvgCreateFont(vg, "sans", "Roboto-Regular.ttf");
    if (fontNormal == -1) {
        printf("Could not add font italic.\n");
        return -1;
    }
    fontBold = nvgCreateFont(vg, "sans-bold", "Roboto-Bold.ttf");
    if (fontBold == -1) {
        printf("Could not add font bold.\n");
        return -1;
    }


    glfwGetCursorPos(window, &mx, &my);
    glfwGetWindowSize(window, &winWidth, &winHeight);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    // Calculate pixel ration for hi-dpi devices.
    pxRatio = (float)fbWidth / (float)winWidth;
    
    glViewport(0, 0, fbWidth, fbHeight);
    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    printf("GL_COLOR_BUFFER_BIT: %d\n", GL_COLOR_BUFFER_BIT);
    printf("GL_DEPTH_BUFFER_BIT: %d\n", GL_DEPTH_BUFFER_BIT);
    printf("GL_STENCIL_BUFFER_BIT: %d\n", GL_STENCIL_BUFFER_BIT);

    //drawWindow(vg, "Widgets `n Stuff", 50, 50, 300, 400);

    MemoryManager::Instance()->imageRead(fp);
    MemoryManager::Instance()->garbageCollect();

    initCommonSymbols();
#if defined TW_ENABLE_FFI
    initFFISymbols();
#endif

    /* Store the vg pointer into the globals */
    nameTableInsert(symbols, strHash("vg"), createSymbol("vg"), newCPointer(vg));
    nameTableInsert(symbols, strHash("winWidth"), createSymbol("winWidth"), newInteger(winWidth));
    nameTableInsert(symbols, strHash("winHeight"), createSymbol("winHeight"), newInteger(winHeight));
    nameTableInsert(symbols, strHash("fbWidth"), createSymbol("fbWidth"), newInteger(fbWidth));
    nameTableInsert(symbols, strHash("fbHeight"), createSymbol("fbHeight"), newInteger(fbHeight));
    nameTableInsert(symbols, strHash("pxRatio"), createSymbol("pxRatio"), newFloat(pxRatio));
    nameTableInsert(symbols, strHash("window"), createSymbol("window"), newCPointer(window));
    nameTableInsert(symbols, strHash("fontNormal"), createSymbol("fontNormal"), newInteger(fontNormal));
    nameTableInsert(symbols, strHash("fontBold"), createSymbol("fontBold"), newInteger(fontBold));

    runCode("ObjectMemory changed: #returnFromSnapshot");
    firstProcess = globalSymbol("systemProcess");
    if (firstProcess == nilobj) 
    {
        sysError("no initial process","in image");
        exit(1); 
        return 1;
    }

    /* execute the main system process loop repeatedly */
    /* debugging = true;*/

    while (execute(firstProcess, 15000)) ;

    glfwDestroyWindow(window);
    glfwTerminate();

    /* exit and return - belt and suspenders, but it keeps lint happy */
    exit(0); 
    return 0;
}
