package es.ujaen.entity;

import java.util.Iterator;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class UserNodeRelationsInfo {
	
	private static final Logger logger = LoggerFactory.getLogger(UserNodeRelationsInfo.class);
	
	long id;
	
	UserInfo userInfo;
	
	NodeInfo masterNode;
	
	List<NodeInfo> nodes;
	
	String authName;
	
	String authPass;
	
	public UserNodeRelationsInfo(long id, UserInfo userInfo, NodeInfo masterNode, List<NodeInfo> nodes, String authName, String authPass) {
		super();
		this.id = id;
		this.userInfo = userInfo;
		this.masterNode = masterNode;
		this.nodes = nodes;
		this.authName = authName;
		this.authPass = authPass;
	}
	
	public UserNodeRelationsInfo() {
		super();
	}

	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public UserInfo getUserInfo() {
		return userInfo;
	}

	public void setUserInfo(UserInfo userInfo) {
		this.userInfo = userInfo;
	}

	public List<NodeInfo> getNodes() {
		return nodes;
	}

	public void setNodes(List<NodeInfo> nodes) {
		this.nodes = nodes;
	}

	public String getAuthName() {
		return authName;
	}

	public void setAuthName(String authName) {
		this.authName = authName;
	}
	
	public String getAuthPass() {
		return authPass;
	}

	public void setAuthPass(String authPass) {
		this.authPass = authPass;
	}
	
	public NodeInfo getMasterNode() {
		return masterNode;
	}

	public void setMasterNode(NodeInfo masterNode) {
		this.masterNode = masterNode;
	}

	public String getNodeIdsString() {
		String s = "";
		
		Iterator<NodeInfo> iterator = nodes.iterator();
		
		while(iterator.hasNext()) {
			s += UserNodeRelations.NODES_SPLIT + iterator.next().getId(); 
		}
		
		
		s+=UserNodeRelations.NODES_SPLIT;
		
		return s;
	}
	
	public boolean checkIfNodeIsInNodeList(NodeInfo n) {
		String s = getNodeIdsString();
		return s.contains(UserNodeRelations.NODES_SPLIT + n.getId() + UserNodeRelations.NODES_SPLIT);
	}

	@Override
	public String toString() {
		return "UserNodeRelationsInfo [id=" + id + ", userInfo=" + userInfo + ", masterNode=" + masterNode + ", nodes="
				+ nodes + ", authName=" + authName + ", authPass=" + authPass + "]";
	}
		
	
	
}
