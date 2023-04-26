package es.ujaen.utils;

import java.security.SecureRandom;

public class PaddingUtil {
	public static final String RANDOMPATTERN1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	
	public static String StaticPaddingLeft(int length, char padChar, String inputString) {
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < length; i++) {
			sb.append(padChar);
		}
		sb.append(inputString);
		
		return sb.toString();
	}
	
	public static String StaticPaddingRight(int length, char padChar, String inputString) {
		StringBuilder sb = new StringBuilder();
		sb.append(inputString);
		for(int i = 0; i < length; i++) {
			sb.append(padChar);
		}
		
		return sb.toString();
	}
	
	public static String StaticPadding(int length, char padChar) {
		StringBuilder sb = new StringBuilder();
		for(int i = 0; i < length; i++) {
			sb.append(padChar);
		}
		
		return sb.toString();
	}
	
	public static String RandomPaddingRight(int maxLength, String pattern, String inputString) {
		StringBuilder sb = new StringBuilder();
		sb.append(inputString);
		
		byte[] random = new byte[1];
        new SecureRandom().nextBytes(random);
		
        int padLength = (random[0]+255) % maxLength;	//se suman 255 para quitar los valores negativos de los bytes
        
        byte[] randomPattern = new byte[padLength];
        new SecureRandom().nextBytes(randomPattern);
        
		for(int i = 0; i < padLength; i++) {
			int randomIndex = (randomPattern[i]+255)%pattern.length();
			char randomChar = pattern.charAt(randomIndex);
			sb.append(randomChar);
		}
		
		return sb.toString();
	}
	
	public static String RandomPaddingLeft(int maxLength, String pattern, String inputString) {
		StringBuilder sb = new StringBuilder();
		
		byte[] random = new byte[1];
        new SecureRandom().nextBytes(random);
		
        int padLength = (random[0]+255) % maxLength;	//se suman 255 para quitar los valores negativos de los bytes
        
        byte[] randomPattern = new byte[padLength];
        new SecureRandom().nextBytes(randomPattern);
        
		for(int i = 0; i < padLength; i++) {
			int randomIndex = (randomPattern[i]+255)%pattern.length();
			char randomChar = pattern.charAt(randomIndex);
			sb.append(randomChar);
		}
		sb.append(inputString);

		return sb.toString();
	}
	
	public static String RandomPadding(int maxLength, String pattern) {
		StringBuilder sb = new StringBuilder();
		
		byte[] random = new byte[1];
        new SecureRandom().nextBytes(random);
		
        int padLength = (random[0]+255) % maxLength;	//se suman 255 para quitar los valores negativos de los bytes
        
        byte[] randomPattern = new byte[padLength];
        new SecureRandom().nextBytes(randomPattern);
        
		for(int i = 0; i < padLength; i++) {
			int randomIndex = (randomPattern[i]+255)%pattern.length();
			char randomChar = pattern.charAt(randomIndex);
			sb.append(randomChar);
		}
		
		return sb.toString();
	}
}
