#pragma once
#ifndef __gl_debug_h__
#define __gl_debug_h__

// STL
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <set>


// OpenGL
#include <gl/gl_glew.h>


namespace OpenGL
{


class CContext
{
public:
   
    enum class TDebugLevel
    {
        off,
        all,
        error_only
    };

    inline void introspection();
    inline void log_context_information();
    inline void init_debug_output();
    inline void init(TDebugLevel debug_level);
    inline void Init(TDebugLevel debug_level) { init(debug_level);}

    static inline void GLAPIENTRY DebugCallbackHandler( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam );
    inline void debug_callback( unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message );

private:

    TDebugLevel _debug_level = TDebugLevel::all;
    GLint major_version = 0;
    GLint minor_version = 0;
    GLint contex_mask = 0;
    std::set<std::string> extensions;
    bool khr_debug = false;
    bool arb_debug_output = false;
    bool amd_debug_output = false;
    bool debug_output_supported = false;
};

void CContext::introspection()
{
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contex_mask);

    GLint no_of_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &no_of_extensions);
    for (int i = 0; i < no_of_extensions; ++i)
        extensions.insert((const char*)glGetStringi(GL_EXTENSIONS, i));

    // KHR_debug
    // https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt
    khr_debug = extensions.find("GL_KHR_debug") != extensions.end();

    // ARB_debug_output
    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_debug_output.txt
    arb_debug_output = extensions.find("GL_ARB_debug_output") != extensions.end();

    // AMD_debug_output
    // https://www.khronos.org/registry/OpenGL/extensions/AMD/AMD_debug_output.txt
    amd_debug_output = extensions.find("GL_AMD_debug_output") != extensions.end();

    debug_output_supported =
        major_version > 4 ||
        (major_version == 4 && minor_version >= 3) ||
        khr_debug || arb_debug_output || amd_debug_output;
}

void CContext::log_context_information()
{
    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << glGetString(GL_RENDERER) << std::endl;
    std::cout << glGetString(GL_VERSION) << std::endl;
    std::cout << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << "context: " << major_version << "." << minor_version << " ";
    if (contex_mask & GL_CONTEXT_CORE_PROFILE_BIT)
      std::cout << "core";
    else if (contex_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
      std::cout << "compatibility";
    if (contex_mask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
      std::cout << ", forward compatibility";
    if (contex_mask & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT)
      std::cout << ", robust access";
    if (contex_mask & GL_CONTEXT_FLAG_DEBUG_BIT)
      std::cout << ", debug";
    if (debug_output_supported)
        std::cout << ", debug output";
    std::cout << std::endl;

    static bool log_extensions = false;
    if (log_extensions) {
        std::cout << std::to_string(extensions.size()) + " extensions" << std::endl;
        for (auto &extension_name: extensions)
            std::cout << extension_name << std::endl;
    }
}

void CContext::DebugCallbackHandler(
    unsigned int  source,     //!< I - 
    unsigned int  type,       //!< I - 
    unsigned int  id,         //!< I - 
    unsigned int  severity,   //!< I - 
    int           length,     //!< I - length of debug message
    const char* message,    //!< I - debug message
    const void* userParam) //!< I - user parameter
{
    if (userParam == nullptr)
        return;
    CContext* context_ptr = static_cast<CContext*>(const_cast<void*>(userParam));
    context_ptr->debug_callback(source, type, id, severity, length, message);
}

void CContext::init_debug_output() 
{
    if (!debug_output_supported)
        return;
    if (_debug_level == TDebugLevel::off)
        return;
    if (!GLEW_KHR_debug)
        return;

    glDebugMessageCallback(&CContext::DebugCallbackHandler, this);

    // See also [How to use glDebugMessageControl](https://stackoverflow.com/questions/51962968/how-to-use-gldebugmessagecontrol/51963554#51963554)
    switch (_debug_level)
    {
    default:
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        break;

    case TDebugLevel::error_only:
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_FALSE);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
        break;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    std::string debug_message = "starting debug messaging service";
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, (GLsizei)debug_message.size(), debug_message.c_str());
}

void CContext::init(CContext::TDebugLevel debug_level)
{
    _debug_level = debug_level;

    introspection();
    log_context_information();
    init_debug_output();

    std::cout << std::endl;
}

void CContext::debug_callback(
    unsigned int  source,   //!< I - 
    unsigned int  type,     //!< I - 
    unsigned int  id,       //!< I - 
    unsigned int  severity, //!< I - 
    int           length,   //!< I - length of debug message
    const char   *message ) //!< I - debug message
{
    static const std::vector<GLenum> error_ids
    {
        GL_INVALID_ENUM,
        GL_INVALID_VALUE,
        GL_INVALID_OPERATION,
        GL_STACK_OVERFLOW,
        GL_STACK_UNDERFLOW,
        GL_OUT_OF_MEMORY
    };

    auto error_it = std::find( error_ids.begin(), error_ids.end(), id );
    if (error_it != error_ids.end())
        std::cout << "debug error: " << message << std::endl << std::endl;
    else
        std::cout << "debug output: " << message << std::endl << std::endl;
}

} // OpenGL


#endif // __gl_debug_h__
