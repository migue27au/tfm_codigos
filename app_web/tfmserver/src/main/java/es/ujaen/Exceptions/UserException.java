package es.ujaen.Exceptions;

import java.io.IOException;

public class UserException extends IOException{
	public final static int USER_NOT_FOUND = 100;
	public final static int USER_NOT_OWNS_SPECIFIED_NODE = 200;
	public final static int IP_BLOCKED = 300;
	public final static int USER_BLOCKED = 400;
	
	
	private int type;


	public UserException(int type) {
		super();
		this.type = type;
	}


	public int getType() {
		return type;
	}
	
	@Override
	public String getMessage() {
		switch(type) {
		case USER_NOT_OWNS_SPECIFIED_NODE:
			return "Specified node don't belong to specified user";
		case USER_NOT_FOUND:
			return "Specified user was not found";
		case IP_BLOCKED:
			return "Client IP address is blocked due to too many attempts";
		case USER_BLOCKED:
			return "User account have been block due to malicious activity";
		default:
			return super.getMessage();
		}
	}
	
	
	
}
