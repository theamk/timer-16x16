Import("env")
import shutil
import os

# Per ~/.platformio/platforms/raspberrypi/builder/main.py, the "uf2" file is not registered
# as proper SCons target, but rather is created in post-action on elf target. So the best we can
# is to register post-action on same target, and hope they won't ever get reordered.
def copyout_uf2(source, target, env):
    uf2_file = target[0].get_path().replace(".elf", ".uf2")
    print("Copying out to project root", uf2_file)
    shutil.copy(uf2_file, os.path.basename(uf2_file))

env.AddPostAction("${PROGPATH}", copyout_uf2)


# Create .lst file during build process, based on
# https://community.platformio.org/t/how-to-create-a-lst-file-using-objdump-during-the-build-process/34632/5
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "arm-none-eabi-objdump", "-d", "-S", "--demangle",
        "$BUILD_DIR/${PROGNAME}.elf", ">", "$BUILD_DIR/${PROGNAME}.lst"
    ]), "Building $BUILD_DIR/${PROGNAME}.lst")
)
