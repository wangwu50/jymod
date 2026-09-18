# 双 Lua 1.11

同一 APK 内置 Lua 5.2 和 Lua 5.4，启动器按 MOD 选择运行时。

## 使用

正式版包名 `com.wangwu.jymod`，应用名“金庸mod启动器”，versionCode 4 / versionName 1.11。使用独立发布签名；覆盖安装要求包名、签名证书相同且版本号不低于旧包。双 Lua 预览版使用不同包名，可共存。

将适用于 jymod 的 MOD 放在内部存储 `/jymod/`。按应用界面申请的存储权限自行授权，然后点击 MOD 直接启动。版本优先级：长按手动保存的选择 > info.txt 中的 [LUA]=LUA52/LUA54 > 文件夹前缀 [52]/[54] > 默认5.2。需要调整时长按 MOD，选择版本并开始；按 MOD 路径记忆手动选择，不自动覆盖为推断值。控制方式继续支持有按键/无按键。原包与测试包会读取同一外部 MOD 目录，因此使用实际游戏前请备份进度。

这不是群侠集结 APK，不能直接用此前专门适配群侠集结的 heishan 包来检验 jymod 兼容性；应使用原 jymod 数据包。更换运行时不会转换 Lua 字节码或 MOD API。

## 实现

- 原改造版 LUA52.zip/LUA54.zip 各自展开到 lua52/lua54，分别生成 liblua52.so/liblua54.so。
- 共用 src/engine.mk，将同一组 C++ 源码分别按对应头文件编译为 main52/main54，链接各自 Lua。
- 公共 SDL、字体、图片、音频、ZIP 和 C++ 库共享打包。
- Lua52Activity/Lua54Activity 为私有 Activity，分别位于 :lua52/:lua54 进程。
- 原 JNI 声明仍保留在 JYmodActivity。loadLibraries 阶段先获取文件锁，再加载正确库、设置游戏路径和控制模式，之后才进入 SDL 窗口/线程初始化。
- 同一应用内所有 MOD 共用一个操作系统文件锁，避免两种运行时同时开游戏。锁随进程退出/崩溃释放；不能阻止另一独立安装的旧版启动器同时访问同一个 MOD。
- 保留 SDL 正常销毁时的 System.exit，作用域已隔离为游戏进程，主启动器不退出。修正 SDL 库加载失败弹窗在 mSingleton 尚未初始化时的空引用。
- 首次无存储权限时不再因目录列表为空而崩溃；取得权限后刷新列表。MOD 选择不再修改文件访问时间。

## 构建

当前发行包仅 ARM64。当前验证环境：Gradle 8.9、AGP 8.2.2、Android SDK 34、NDK 27.1.12297006、Android Studio 自带 JBR。minSdk 21 / targetSdk 28 沿用旧式文件访问兼容策略，不是商店上架配置。

设置 local.properties 中 sdk.dir，并配置 JAVA_HOME 后执行：

```
./gradlew :app:assembleDebug
python3 tests/verify_apk.py
```

为适配本机 NDK，对旧依赖做了小范围修正：SDL glShaderSource 的 const 指针声明；zlib HAVE_UNISTD_H；minizip 不再在 Android 上定义 libc 内部宏，移除未被游戏使用的 CLI 示例源码；启用 c++_shared。

## 验证结果与边界

- assembleDebug 成功。
- APK 内两套 Lua/main 产物齐全；检查 ELF DT_NEEDED，各 main 只依赖其对应 Lua；SDL_main 和原 JNI 入口存在。
- ARM64 Android 模拟器实跑 SDL/JNI/Lua 5.2 → 5.4 → 5.2；5.2 bit32、5.4 整除、实际 _VERSION、正常退出、文件锁释放通过。
- 测试使用应用私有目录，无需授予 MANAGE_EXTERNAL_STORAGE。运行命令：`adb shell am instrument -w com.wangwu.jymod/com.wangwu.jymod.RuntimeSmokeInstrumentation`。该 instrumentation 和测试脚本只编入 debug 构建。
- 完整 MOD 的中文变量、特殊字节码、存读档、战斗、音频、外部存储授权流程仍需实机验证；未承诺所有 MOD 跨版本可用。
- 安装和升级不会主动转换 MOD 或玩家存档；签名不同的旧包无法直接覆盖。

## 启动页与版本识别

启动页在 Manifest 与 Activity 两处设置横屏，恢复原布局及副标题“由美好的地狱倾情打造”。控制方式收起时显示文字，展开显示选项。普通点击直接启动，长按手动选择并记忆。

`tests/LuaRuntimeChoiceTest.java` 验证版本推断优先级、错误值及默认值。

## 发布构建与签名

```sh
./gradlew :app:assembleRelease
export ANDROID_SDK_ROOT=/path/to/Android/sdk
export JAVA_HOME=/path/to/jdk
export JYMOD_KEYSTORE=/private/path/jymod-release.jks
export JYMOD_PASSWORD_FILE=/private/path/store-password.txt
# 默认 alias 为 jymod-release；密码文件为单行，store/key 密码相同。
tools/sign-release.sh app/build/outputs/apk/release/app-release-unsigned.apk /path/to/jymod-1.11-arm64.apk
python3 tests/verify_apk.py /path/to/jymod-1.11-arm64.apk
```

发布签名、密码和 APK 不存入仓库。妥善备份 keystore 和密码；后续升级需继续使用同一签名。Release 构建不包含 debug instrumentation / 测试资源，未开启 debuggable，不使用 debug 签名。

本包包名为 com.wangwu.jymod，可与旧 com.wangwu.jymod52 / com.wangwu.jymod54 及预览版 com.wangwu.jymod.dualpreview 共存。旧 APK 的调试签名不同，且两个旧包包名不同，不属于本包的覆盖升级对象。多个启动器读取同一 MOD 外部目录，避免同时运行同一个 MOD。

## 1.7.1 最近游玩排序

启动 MOD 后记录独立的持久化顺序号，最近启动的 MOD 排在列表顶部；普通点击与长按选择 Lua 后启动共用记录逻辑。返回首页及重启应用时重新排序。未记录的 MOD 按名称排序。仅打开版本选择框或取消不会更改顺序。不依赖设备时钟或修改 MOD 文件的访问时间。1.7 未保存游玩历史，因此升级后需启动一次 MOD 才会记录其新顺序。

恢复无按键模式的原操作说明和“不再提示”选项，沿用原偏好键；切换控制方式后立即记忆，取消或尚未启动 MOD 也会保留设置。

## 1.11 版本号更正

旧的独立 Lua 5.2 / 5.4 应用已经发布到 1.10；双 Lua 初版误沿用仓库旧版本号，标为了 1.7 / 1.7.1。本版统一更正为 1.11，versionCode 升为 4。包名和发布签名不变，可覆盖升级双 Lua 正式版 1.7 / 1.7.1，保留设置和游玩排序记录。游戏逻辑未改动。
