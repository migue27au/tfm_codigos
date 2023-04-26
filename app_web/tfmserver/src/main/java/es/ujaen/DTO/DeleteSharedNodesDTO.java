package es.ujaen.DTO;

import java.util.List;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class DeleteSharedNodesDTO {
	
	private static final Logger logger = LoggerFactory.getLogger(DeleteSharedNodesDTO.class);

	
	List<Long> usersList;
	
	long nodeId;

	
	
	public DeleteSharedNodesDTO() {
		super();
	}



	public DeleteSharedNodesDTO(List<Long> usersList, long nodeId) {
		super();
		this.usersList = usersList;
		this.nodeId = nodeId;
	}



	public List<Long> getUsersList() {
		return usersList;
	}



	public void setUsersList(List<Long> usersList) {
		this.usersList = usersList;
	}



	public long getNodeId() {
		return nodeId;
	}



	public void setNodeId(long nodeId) {
		this.nodeId = nodeId;
	}



	@Override
	public String toString() {
		String s = "DeleteShareNodeDTO [usersList=(";
		for(int i = 0; i < usersList.size(); i++) {
			s += usersList.get(i) +",";
		}
		s += "), node=" + nodeId + "]";
		return s;
	}

	


}
