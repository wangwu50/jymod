from pathlib import Path
import zipfile,subprocess,json,sys,os,shutil,tempfile
root=Path(__file__).resolve().parents[1]
apk=Path(sys.argv[1]) if len(sys.argv)>1 else root/'app/build/outputs/apk/debug/app-debug.apk'
sdk=Path(os.environ.get('ANDROID_SDK_ROOT', os.environ.get('ANDROID_HOME', str(Path.home()/'Library/Android/sdk'))))
readelf=os.environ.get('LLVM_READELF') or shutil.which('llvm-readelf')
if not readelf:
    candidates=list((sdk/'ndk/27.1.12297006/toolchains/llvm/prebuilt').glob('*/bin/llvm-readelf*'))
    if not candidates: raise SystemExit('Set LLVM_READELF or ANDROID_SDK_ROOT to locate llvm-readelf')
    readelf=candidates[0]
output=root/'artifacts';output.mkdir(exist_ok=True)
expected_abis = {
    'armeabi-v7a': ('ELF32', 'ARM'),
    'arm64-v8a': ('ELF64', 'AArch64'),
    'x86': ('ELF32', 'Intel 80386'),
    'x86_64': ('ELF64', 'Advanced Micro Devices X86-64'),
}
with zipfile.ZipFile(apk) as z, tempfile.TemporaryDirectory() as temp:
    names=set(z.namelist())
    actual_abis={n.split('/')[1] for n in names if n.startswith('lib/') and n.endswith('.so')}
    assert actual_abis == set(expected_abis), actual_abis
    results={}
    for abi, (elf_class, machine) in expected_abis.items():
        prefix='lib/'+abi+'/'
        required=['lua52','lua54','main52','main54','SDL2','SDL2_image','SDL2_ttf','bass','zlib','c++_shared']
        assert all(prefix+'lib'+s+'.so' in names for s in required), abi
        assert prefix+'liblua.so' not in names, abi
        results[abi]={}
        for name in sorted(n for n in names if n.startswith(prefix) and n.endswith('.so')):
            path=Path(temp)/Path(name).name
            path.write_bytes(z.read(name))
            header=subprocess.check_output([str(readelf),'-h',str(path)],text=True)
            assert elf_class in header and machine in header, name
            if path.name not in ('libmain52.so','libmain54.so'): continue
            version=path.name[7:9]
            dynamic=subprocess.check_output([str(readelf),'-d',str(path)],text=True)
            assert '[liblua'+version+'.so]' in dynamic, name
            other='54' if version=='52' else '52'
            assert '[liblua'+other+'.so]' not in dynamic, name
            symbols=subprocess.check_output([str(readelf),'--dyn-syms',str(path)],text=True)
            assert 'SDL_main' in symbols and 'Java_com_wangwu_jymod_JYmodActivity_nativeSetGamePath' in symbols, name
            results[abi][version]=[line.strip() for line in dynamic.splitlines() if 'NEEDED' in line]
    (output/'native-validation.json').write_text(json.dumps(results,indent=2))
print('PASS all four ABIs: ELF architecture, both Lua runtimes, matching engine dependencies and SDL/JNI entry points')
