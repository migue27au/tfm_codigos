package es.ujaen.Exceptions;

import java.io.IOException;

public class UserNodeRelationsException extends IOException{
	public final static int USER_NODE_RELATIONS_NOT_FOUND = 100;
	public final static int USER_NODE_RELATIONS_EXISTS = 101;
	public final static int BAD_AUTHENTICATION = 200;
	public final static int NODE_ID_NOT_FOUND_IN_NODE_IDS = 300;
	
	
	private int type;


	public UserNodeRelationsException(int type) {
		super();
		this.type = type;
	}


	public int getType() {
		return type;
	}
	
	@Override
	public String getMessage() {
		switch(type) {
		case USER_NODE_RELATIONS_NOT_FOUND:
			return "Specified userNodeRelations was not found";
		case USER_NODE_RELATIONS_EXISTS:
			return "Specified userNodeRelations exists";
		case BAD_AUTHENTICATION:
			return "Authentication process failed";
		case NODE_ID_NOT_FOUND_IN_NODE_IDS:
			return "Specified nodeId doesn't found in the nodeIds array";	
		default:
			return super.getMessage();
		}
	}
	
	
	
}
