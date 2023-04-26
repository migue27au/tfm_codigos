package es.ujaen.Exceptions;

import java.io.IOException;

public class DataException extends IOException{
	public final static int NO_HASH_FOUND = 100;
	public final static int HASH_FOUND = 101;
	private int type;


	public DataException(int type) {
		super();
		this.type = type;
	}


	public int getType() {
		return type;
	}
	
	@Override
	public String getMessage() {
		switch(type) {
		case NO_HASH_FOUND:
			return "Specified hash was not found";
		case HASH_FOUND:
			return "Specified hash was found";
		default:
			return super.getMessage();
		}
	}
	
	
	
}
