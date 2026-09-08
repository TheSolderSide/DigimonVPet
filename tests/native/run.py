"""Run real game logic on the host using a local Zig C++ compiler."""
from pathlib import Path
import os
import subprocess

root = Path(__file__).resolve().parents[2]
build = root / '.pio' / 'native-tests'
build.mkdir(parents=True, exist_ok=True)
compiler = root / '.pio' / 'test-tools' / 'ziglang' / 'zig.exe'
env = dict(os.environ, ZIG_GLOBAL_CACHE_DIR=str(build / 'zig-cache'))
sources = ['tests/native/care_tests.cpp', 'src/GameLogic/Digimon.cpp',
           'src/GameLogic/EvolutionHandler.cpp', 'src/SaveGame/SaveGameHandler.cpp',
           'src/VPetLCD/Screens/AnimationScreens/TrainingAnimationScreen.cpp']
executable = build / 'care-v1.exe'
with (build / 'compile.log').open('w') as log:
    subprocess.run([str(compiler), 'c++', '-std=c++17', '-Wno-writable-strings',
                    '-Itests/native/stubs', *sources, '-o', str(executable)],
                   cwd=root, env=env, check=True, stdout=log, stderr=log)
subprocess.run([str(executable)], cwd=root, check=True)
