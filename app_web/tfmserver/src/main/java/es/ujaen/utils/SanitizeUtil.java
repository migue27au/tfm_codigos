package es.ujaen.utils;
import org.apache.commons.text.StringEscapeUtils;

public class SanitizeUtil {
	public static String toHTMLEncode(String text) {
		return StringEscapeUtils.escapeHtml4(text);
	}
	
	public static boolean checkPasswordPolicy(String text) {
		String regex = "^(?=.*[0-9])(?=.*[a-z\u00f1])(?=.*[A-Z\u00d1])(?=.*[ !@#&()–[{}]:;',?/*~$^+=<>]).{10,}$";
		return text.matches(regex);
	}
}
