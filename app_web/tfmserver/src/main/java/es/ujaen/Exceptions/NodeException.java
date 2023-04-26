package es.ujaen.Exceptions;

import java.io.IOException;

public class NodeException extends IOException{
	public final static int NO_NODENAME_FOUND = 100;
	public final static int NODE_NOT_FOUND = 200;
	public final static int NODE_EXISTS = 201;
	public final static int SPECIFIED_NODE_ALREADY_SHARED_WITH_USER = 300;
	public final static int BAD_AUTHENTICATION = 400;
	public final static int TWO_NODES_WITH_SAME_NODENAME = 500;
	public final static int TWO_NODES_WITH_SAME_MAC = 600;
	
	
	
	private int type;


	public NodeException(int type) {
		super();
		this.type = type;
	}


	public int getType() {
		return type;
	}
	
	@Override
	public String getMessage() {
		switch(type) {
		case NO_NODENAME_FOUND:
			return "Specified nodename was not found";
		case SPECIFIED_NODE_ALREADY_SHARED_WITH_USER:
			return "Specified node is already shared with specified shared";
		case NODE_EXISTS:
			return "Specified node exists";
		case NODE_NOT_FOUND:
			return "Specified node was not found";
		case BAD_AUTHENTICATION:
			return "Authentication process failed";
		case TWO_NODES_WITH_SAME_NODENAME:
			return "Two nodes have the same nodename";
		case TWO_NODES_WITH_SAME_MAC:
			return "Two nodes have the same mac";
		default:
			return super.getMessage();
		}
	}
	
	
	
}
