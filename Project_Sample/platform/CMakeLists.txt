set(PLATFORM_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Startup File
set(STARTUP_SRC ${PLATFORM_ROOT}/bsp/startup_stm32f10x_md.s)

# SPL Sources (Standard Peripheral Library)
file(GLOB SPL_SOURCES 
    "${PLATFORM_ROOT}/spl/src/*.c"
)

# CMSIS System Initialization (Use stm32f10x template or custom)
# Note: system_stm32f10x.c typically resides in cmsis or project root.
# For now, we omit it if not strictly found, or assume it's in bsp/cmsis if available.
# Checking confirmed bsp/cmsis only has headers. logic needs to be robust.
# If system_stm32f10x.c is missing, we might need to add it or exclude it.
# Let's assume we might need to generate it or it's implicitly handled?
# Wait, user image showed structure... let's stick to what we verify.

add_library(platform STATIC
    ${STARTUP_SRC}
    ${SPL_SOURCES}
)

target_include_directories(platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PLATFORM_ROOT}/spl/inc
    ${PLATFORM_ROOT}/bsp/cmsis
)

# Export Linker Script Variable
set(LINKER_SCRIPT ${PLATFORM_ROOT}/bsp/linker/stm32f103.ld PARENT_SCOPE)

# Link options associated with this platform
target_link_options(platform INTERFACE
    -T${PLATFORM_ROOT}/bsp/linker/stm32f103.ld
    -mcpu=cortex-m3
    -mthumb
)

target_compile_options(platform PUBLIC
    -mcpu=cortex-m3
    -mthumb
    -DSTM32F10X_MD
    -DUSE_STDPERIPH_DRIVER
)
