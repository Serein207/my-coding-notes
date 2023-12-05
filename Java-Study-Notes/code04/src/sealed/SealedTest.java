package sealed;

import java.util.*;

enum JSONBoolean implements JSONPrimitive {
  FALSE, TRUE;

  @Override
  public String toString() {
    return super.toString().toLowerCase();
  }
}

enum JSONNull implements JSONPrimitive {
  INSTANCE;

  @Override
  public String toString() {
    return "null";
  }
}

sealed interface JSONValue permits JSONArray, JSONObject, JSONPrimitive {
  public default String type() {
    if (this instanceof JSONArray) return "array";
    else if (this instanceof JSONObject) return "object";
    else if (this instanceof JSONNumber) return "number";
    else if (this instanceof JSONString) return "string";
    else if (this instanceof JSONBoolean) return "boolean";
    else return "null";
  }
}

sealed interface JSONPrimitive extends JSONValue
        permits JSONNumber, JSONString, JSONBoolean, JSONNull {
}

final class JSONArray extends ArrayList<JSONValue> implements JSONValue {
}

final class JSONObject extends HashMap<String, JSONValue> implements JSONValue {
  @Override
  public String toString() {
    StringBuilder result = new StringBuilder();
    result.append("{");
    for (Map.Entry<String, JSONValue> entry : entrySet()) {
      if (result.length() > 1) result.append(",");
      result.append("\"");
      result.append(entry.getKey());
      result.append("\":");
      result.append(entry.getValue());
    }
    result.append("}");
    return result.toString();
  }
}

final record JSONNumber(double value) implements JSONPrimitive {
  @Override
  public String toString() {
    return "" + value;
  }
}

final record JSONString(String value) implements JSONPrimitive {
  @Override
  public String toString() {
    return "\"" + value.translateEscapes() + "\"";
  }
}

public class SealedTest {
  public static void main(String[] args) {
    JSONObject obj = new JSONObject();
    obj.put("name", new JSONString("Harry"));
    obj.put("salary", new JSONNumber(90000));
    obj.put("married", JSONBoolean.FALSE);
    JSONArray arr = new JSONArray();
    arr.add(new JSONNumber(13));
    arr.add(JSONNull.INSTANCE);

    obj.put("luckyNumbers", arr);
    System.out.println(obj);
    System.out.println(obj.type());
  }
}