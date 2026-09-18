import com.wangwu.jymod.LuaRuntimeChoice;
public class LuaRuntimeChoiceTest {
 static void check(String wanted,String saved,String info,String name) {
  if(!wanted.equals(LuaRuntimeChoice.resolve(saved,info,name)))throw new AssertionError(name);
 }
 public static void main(String[] args) {
  check("54",null,"","[54]黑山群侠传");
  check("52",null,"","[52] 黑山群侠传");
  check("52","52","[LUA]=LUA54","[54]黑山");
  check("54",null,"[LUA]=LUA54\r\n","[52]黑山");
  check("52",null,"[LUA]=LUA52","[54]黑山");
  check("54","bad","\u00ef\u00bb\u00bf[LUA]=LUA54\n","普通mod");
  check("52",null,"","普通mod");
  check("52",null,"-- [LUA]=LUA54","普通mod");
  System.out.println("PASS manual preference, metadata, folder prefix, fallback, malformed values");
 }
}
