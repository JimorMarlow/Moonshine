#pragma once
// version.h - Monshine version control

// NOTE: after version changing run script in terminal:
// > python sync_version.py
// and config files: package.json will be patched...

#define MS_VERSION_MAJOR 1
#define MS_VERSION_MINOR 2
#define MS_VERSION_PATCH 1

// Хелпер для stringify
#define MS_STRINGIFY_HELPER(x) #x
#define MS_STRINGIFY(x) MS_STRINGIFY_HELPER(x)

//Собрать строку версии "X.X.X"
#define MS_VERSION_STRING MS_STRINGIFY(MS_VERSION_MAJOR) "." MS_STRINGIFY(MS_VERSION_MINOR) "." MS_STRINGIFY(MS_VERSION_PATCH)