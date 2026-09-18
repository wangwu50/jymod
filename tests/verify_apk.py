from pathlib import Path
import zipfile,subprocess,json,sys,os,shutil
root=Path(__file__).resolve().parents[1]
apk=Path(sys.argv[1]) if len(sys.argv)>1 else root/'app/build/outputs/apk/debug/app-debug.apk'
sdk=Path(os.environ.get('ANDROID_SDK_ROOT', os.environ.get('ANDROID_HOME', str(Path.home()/'Library/Android/sdk'))))
readelf=os.environ.get('LLVM_READELF') or shutil.which('llvm-readelf')
if not readelf:
    candidates=list((sdk/'ndk/27.1.12297006/toolchains/llvm/prebuilt').glob('*/bin/llvm-readelf*'))
    if not candidates: raise SystemExit('Set LLVM_READELF or ANDROID_SDK_ROOT to locate llvm-readelf')
    readelf=candidates[0]
output=root/'artifacts';output.mkdir(exist_ok=True)
with zipfile.ZipFile(apk) as z:
    names=z.namelist()
    assert all('lib/arm64-v8a/lib'+s+'.so' in names for s in ['lua52','lua54','main52','main54'])
    assert 'lib/arm64-v8a/liblua.so' not in names
    results={}
    for version in ['52','54']:
        path=output/('libmain'+version+'.so');path.write_bytes(z.read('lib/arm64-v8a/'+path.name))
        dynamic=subprocess.check_output([str(readelf),'-d',str(path)],text=True)
        assert '[liblua'+version+'.so]' in dynamic
        other='54' if version=='52' else '52'
        assert '[liblua'+other+'.so]' not in dynamic
        symbols=subprocess.check_output([str(readelf),'--dyn-syms',str(path)],text=True)
        assert 'SDL_main' in symbols and 'Java_com_wangwu_jymod_JYmodActivity_nativeSetGamePath' in symbols
        results[version]=[line.strip() for line in dynamic.splitlines() if 'NEEDED' in line]
    (output/'native-validation.json').write_text(json.dumps(results,indent=2))
print('PASS APK contains both Lua runtimes and matching engine dependencies, SDL/JNI entry points')
