# jymod
金庸mod启动器，可以实现一个apk，运行多个金庸mod的效果。

## 1.7 双 Lua

一个 APK 同时包含 Lua 5.2 / 5.4，不需要解压替换 Lua 或分别打包。点击 MOD 直接启动，长按可以切换版本并记住选择。自动选择优先级：手动记录 > info.txt 的 `[LUA]=LUA52` / `LUA54` > 文件夹 `[52]` / `[54]` 前缀 > 默认 5.2。

构建使用 NDK **27.1.12297006**，已修复旧 SDL、zlib、minizip 的新版 NDK 编译问题；无需限制在 NDK 21。当前发布 ARM64 APK，Gradle 8.9 / AGP 8.2.2 / SDK 34。

保留原 Lua 5.2 的中文变量和 32 位 luac 字节码改造。切换运行时不会转换字节码或 MOD API。构建、签名和验证方法见 [双 Lua 说明](README-DUAL-LUA.md)。

## roadmap
- 支持 proload 贴图的方式，而不是每次更改地图才加载贴图的方式
- [x] 支持切换 Lua 5.2 / 5.4，不用打多个包
- 支持按键、无按键切换
- 支持雨雪天气设置
- 支持picture 按zip读取，图片偏移等操作