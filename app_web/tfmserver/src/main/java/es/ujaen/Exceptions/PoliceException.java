package es.ujaen.Exceptions;

import java.io.IOException;

public class PoliceException extends IOException{
	public final static int USER_TRY_TO_SHARE_OTHERS_NODES = 100;
	public final static int USER_TRY_TO_GET_OTHER_NODE = 200;
	public final static int USER_TRY_TO_DELETE_SHARED_OTHERS_NODES = 300;
	public final static int USER_TRY_TO_EDIT_OTHER_NODE = 400;
	public final static int USER_TRY_TO_DELETE_OTHER_NODE = 500;
	public final static int USER_TRY_TO_EXPORT_OTHER_NODE = 600;
	
	public final static int USER_TRY_TO_GET_MORE_DATA = 700;
	public final static int USER_CAUSE_DATE_PARSE_ERROR = 800;
	
	private int type;


	public PoliceException(int type) {
		super();
		this.type = type;
	}


	public int getType() {
		return type;
	}
	
	@Override
	public String getMessage() {
		switch(type) {
			case USER_TRY_TO_SHARE_OTHERS_NODES:
				return "Specified user try to share others nodes";
			case USER_TRY_TO_DELETE_SHARED_OTHERS_NODES:
				return "Specified user try to delete shared others nodes";
			case USER_TRY_TO_GET_OTHER_NODE:
				return "Specified user try to get other node";
			case USER_TRY_TO_EDIT_OTHER_NODE:
				return "Specified user try to edit other node";
			case USER_TRY_TO_DELETE_OTHER_NODE:
				return "Specified user try to delete other node";
			case USER_TRY_TO_EXPORT_OTHER_NODE:
				return "Specified user try to export other node";
			case USER_TRY_TO_GET_MORE_DATA:
				return "Specified user try to get more datas than maximum";
			case USER_CAUSE_DATE_PARSE_ERROR:
				return "Specified user input wrong date and cause parse exception";
			default:
				return super.getMessage();
		}
	}
	
	
	
}
