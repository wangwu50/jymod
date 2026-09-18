package com.wangwu.jymod;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** Resolution only: never execute a MOD to detect its interpreter. */
public final class LuaRuntimeChoice {
    private static final Pattern DECLARATION=Pattern.compile("(?im)^\\s*\\[LUA\\]\\s*=\\s*LUA(52|54)\\s*$");
    private static final Pattern PREFIX=Pattern.compile("^\\s*\\[(52|54)\\]");
    private LuaRuntimeChoice() {}
    public static String resolve(String saved,String metadata,String name) {
        if("52".equals(saved)||"54".equals(saved)) return saved;
        Matcher declared=DECLARATION.matcher(metadata==null?"":metadata.replace("\u00ef\u00bb\u00bf",""));
        if(declared.find())return declared.group(1);
        Matcher prefix=PREFIX.matcher(name==null?"":name);
        return prefix.find()?prefix.group(1):"52";
    }
}
