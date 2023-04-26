package es.ujaen.utils;

import java.nio.charset.StandardCharsets;

import com.google.common.hash.Hashing;

public class SHA256Util {

	public static String sha256Hex(String s) {
		return Hashing.sha256().hashString(s, StandardCharsets.UTF_8).toString();
	}
	
}
