#!/usr/bin/env python
import os
import sys
import shutil

# -------------------------------------------------------------------------
# CONFIGURATION
# -------------------------------------------------------------------------
LIBRARY_NAME = "morphic_core"      # Name of the library
OUTPUT_DIR = "game/bin/"           # Where to put the binary
CUSTOM_API = "extension_api.json"  # Your custom API dump

# -------------------------------------------------------------------------
# SCONS SETUP
# -------------------------------------------------------------------------

# Inject the path to the custom API JSON.
ARGUMENTS["custom_api_file"] = CUSTOM_API

# Load the build logic from godot-cpp.
env = SConscript("godot-cpp/SConstruct")

# -------------------------------------------------------------------------
# COMPILATION DB (FIX FOR CLANGD)
# -------------------------------------------------------------------------

# We enable the tool manually instead of passing arguments to godot-cpp.
# This generates 'compile_commands.json' for Neovim/LSP.
env.Tool("compilation_db")
cdb_target = env.CompilationDatabase("compile_commands.json")
env.Append(COMPILATIONDB_USE_DATA=True)

# -------------------------------------------------------------------------
# SPEED OPTIMIZATIONS
# -------------------------------------------------------------------------

# 1. SCons internal cache
env.CacheDir(".scons_cache")

# 2. Use all CPU cores
if not env.GetOption("num_jobs"):
    env.SetOption("num_jobs", os.cpu_count())

# 3. Use ccache if available
if shutil.which("ccache"):
    env["CXX"] = "ccache " + env["CXX"]
    env["CC"] = "ccache " + env["CC"]
    print(f"Info: Using ccache for compilation.")

# 4. Use mold linker if available
if shutil.which("mold"):
    env.Append(LINKFLAGS=["-fuse-ld=mold"])
    print(f"Info: Using mold linker.")

# -------------------------------------------------------------------------
# SOURCES & INCLUDES
# -------------------------------------------------------------------------

env.Append(CPPPATH=["src"])

sources = []
for root, dirs, files in os.walk("src"):
    for file in files:
        if file.endswith(".cpp"):
            sources.append(os.path.join(root, file))

if not sources:
    print("ERROR: No .cpp files found in 'src/' directory.")
    sys.exit(1)

# -------------------------------------------------------------------------
# OUTPUT DEFINITION
# -------------------------------------------------------------------------

if env["platform"] == "linux":
    suffix = ".so"
    platform_tag = "linux"
elif env["platform"] == "windows":
    suffix = ".dll"
    platform_tag = "windows"
elif env["platform"] == "macos":
    suffix = ".dylib"
    platform_tag = "macos"

if env["target"] == "template_debug":
    target_tag = "template_debug"
elif env["target"] == "template_release":
    target_tag = "template_release"
else:
    target_tag = "editor"

arch_tag = env["arch"]
output_name = f"{LIBRARY_NAME}.{platform_tag}.{target_tag}.{arch_tag}{suffix}"
output_path = os.path.join(OUTPUT_DIR, output_name)

# -------------------------------------------------------------------------
# COMPILATION
# -------------------------------------------------------------------------

library = env.SharedLibrary(
    target=output_path,
    source=sources
)

Default(library, cdb_target)
