package es.ujaen.entity;

import java.util.Date;

import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.validation.annotation.Validated;

import es.ujaen.utils.AESUtil;

@Entity
@Table(name = "user_node_relations")
public class UserNodeRelations {
	
	private static final Logger logger = LoggerFactory.getLogger(UserNodeRelations.class);

	
	public static final String NODES_SPLIT = ",";
	
	
	@Id
	@GeneratedValue(strategy = GenerationType.AUTO)
	@Column(name = "id", unique=true)
	long id;
	
	@Column(name = "userId")
	long userId;
	
	@Column(name = "nodeId")
	long nodeId;
	
	@Column(name = "nodeIds")
	String nodeIds;
	
	@Column(name = "authName", length=50, unique=true)
	String authName;
	
	@Column(name = "authPass", length=64)
	String authPass;

	
	public UserNodeRelations() {
		super();
	}

	public UserNodeRelations(long id, long userId, long nodeId, String nodeIds, String authName, String authPass) {
		super();
		this.id = id;
		this.userId = userId;
		this.nodeId = nodeId;
		this.nodeIds = nodeIds;
		this.authName = authName;
		this.authPass = authPass;
	}
	
	public UserNodeRelations(long userId, long nodeId, String nodeIds, String authName, String authPass) {
		super();
		this.userId = userId;
		this.nodeId = nodeId;
		this.nodeIds = nodeIds;
		this.authName = authName;
		this.authPass = authPass;
	}
	
	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public long getUserId() {
		return userId;
	}

	public void setUserId(long userId) {
		this.userId = userId;
	}
	
	public long getNodeId() {
		return nodeId;
	}

	public void setNodeId(long nodeId) {
		this.nodeId = nodeId;
	}
	
	public String getNodeIds() {
		return nodeIds;
	}

	public void setNodeIds(String nodeIds) {
		this.nodeIds = nodeIds;
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

	public UserNodeRelations update(UserNodeRelationsInfo userNodeRelationsInfo) {
		this.id = userNodeRelationsInfo.getId();
		this.userId = userNodeRelationsInfo.getUserInfo().getId();
		this.nodeIds = userNodeRelationsInfo.getNodeIdsString();
		this.authName = userNodeRelationsInfo.getAuthName();
		
		return this;
	}
	
	@Override
	public String toString() {
		return "UserNodeRelations [id=" + id + ", userId=" + userId + ", nodeIds=" + nodeIds + ", authName=" + authName
				+ ", authPass=" + authPass + "]";
	}
	
}
