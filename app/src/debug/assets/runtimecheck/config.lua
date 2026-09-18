CONFIG={XScale=18,YScale=9,Zoom=100,MAXCacheNum=64,EnableSound=0,Debug=1,
KeyScale=100,KeyRepeatDelay=150,KeyRePeatInterval=30,JYMain_Lua='',MidSF2=''}
local root=config.GetPath()
function JY_Main()
    if _VERSION=='Lua 5.2' then assert(bit32.band(7,3)==3)
    elseif _VERSION=='Lua 5.4' then assert(assert(load('return 7 // 2'))()==3)
    else error('unexpected runtime: '.._VERSION) end
    local f=assert(io.open(root..'runtime-result.txt','a'))
    f:write(_VERSION..' OK\n');f:close()
    lib.FillColor(0,0,0,0,0x214b68)
    lib.ShowSurface(0)
    lib.Delay(2000)
end
