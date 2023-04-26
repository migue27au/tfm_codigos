package es.ujaen.entity;

import java.util.ArrayList;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import es.ujaen.DTO.NodeInfoDTO;
import es.ujaen.utils.Location;

public class NodeInfo {
	
	private static final Logger logger = LoggerFactory.getLogger(NodeInfo.class);

	
	private long id;
	private long masterId;
	private String alias;
	private String masterAlias;

	private String nodeName;

	private List<UserInfo> sharedUsers;

	private String addressLsb;
	private String addressMsb;
	private int function;

	private UserInfo owner;
	
	private int sampleRate;
	private boolean output;
	
	private String description;
	
	Location location;
	
	public NodeInfo() {
		super();
	}

	public NodeInfo(long id, long masterId, String alias, String masterAlias, String nodeName, long addressLsb,
			long addressMsb, int function, long ownerId, String ownerUsername, int sampleRate, boolean output,
			List<UserInfo> sharedUsers, Location location, String description) {
		super();
		this.id = id;
		this.masterId = masterId;
		this.alias = alias;
		this.masterAlias = masterAlias;
		this.nodeName = nodeName;
		this.addressLsb = Long.toHexString(addressLsb);
		this.addressMsb = Long.toHexString(addressMsb);
		this.function = function;
		this.owner = new UserInfo(ownerId, ownerUsername);
		this.sampleRate = sampleRate;
		this.output = output;
		this.location = location;
		this.description = description;
	}

	public NodeInfo(Node node, UserInfo owner) {
		super();
		this.id = node.getId();
		this.masterId = node.getMasterId();
		this.alias = node.getAlias();
		this.nodeName = node.getNodeName();
		this.addressLsb = Long.toHexString(node.getAddressLsb());
		this.addressMsb = Long.toHexString(node.getAddressMsb());
		this.function = node.getFunction();
		this.owner = owner;
		this.sampleRate = node.getSampleRate();
		this.output = node.isOutput();
		this.location = new Location(node.getLatitude(),node.getLongitude(), node.getTimeZone());
		this.description = node.getDescription();
	}

	public NodeInfo(Node node, UserInfo owner, List<UserInfo> sharedUsers) {
		super();
		this.id = node.getId();
		this.masterId = node.getMasterId();
		this.alias = node.getAlias();
		this.nodeName = node.getNodeName();
		this.addressLsb = Long.toHexString(node.getAddressLsb());
		this.addressMsb = Long.toHexString(node.getAddressMsb());
		this.function = node.getFunction();
		this.owner = owner;
		this.sampleRate = node.getSampleRate();
		this.output = node.isOutput();
		this.sharedUsers = sharedUsers;
		this.location = new Location(node.getLatitude(),node.getLongitude(), node.getTimeZone());
		this.description = node.getDescription();
	}

	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}


	public long getMasterId() {
		return masterId;
	}


	public void setMasterId(long masterId) {
		this.masterId = masterId;
	}


	public String getAlias() {
		return alias;
	}


	public void setAlias(String alias) {
		this.alias = alias;
	}


	public String getMasterAlias() {
		return masterAlias;
	}


	public void setMasterAlias(String masterAlias) {
		this.masterAlias = masterAlias;
	}


	public String getNodeName() {
		return nodeName;
	}


	public void setNodeName(String nodeName) {
		this.nodeName = nodeName;
	}


	public String getAddressLsb() {
		return addressLsb;
	}


	public void setAddressLsb(String addressLsb) {
		this.addressLsb = addressLsb;
	}


	public String getAddressMsb() {
		return addressMsb;
	}


	public void setAddressMsb(String addressMsb) {
		this.addressMsb = addressMsb;
	}


	public int getFunction() {
		return function;
	}


	public void setFunction(int function) {
		this.function = function;
	}

	public List<UserInfo> getSharedUsers() {
		return sharedUsers;
	}


	public void setSharedUsers(List<UserInfo> sharedUsers) {
		this.sharedUsers = sharedUsers;
	}


	public UserInfo getOwner() {
		return owner;
	}


	public void setOwner(UserInfo owner) {
		this.owner = owner;
	}


	public int getSampleRate() {
		return sampleRate;
	}


	public void setSampleRate(int sampleRate) {
		this.sampleRate = sampleRate;
	}


	public boolean isOutput() {
		return output;
	}


	public void setOutput(boolean output) {
		this.output = output;
	}

	
	public Location getLocation() {
		return location;
	}


	public void setLocation(Location location) {
		this.location = location;
	}


	public String getDescription() {
		return description;
	}


	public void setDescription(String description) {
		this.description = description;
	}


	@Override
	public String toString() {
		return "NodeInfo [id=" + id + ", masterId=" + masterId + ", alias=" + alias + ", masterAlias=" + masterAlias
				+ ", nodeName=" + nodeName + ", sharedUsers=" + sharedUsers + ", addressLsb=" + addressLsb
				+ ", addressMsb=" + addressMsb + ", function=" + function + ", owner=" + owner + ", sampleRate="
				+ sampleRate + ", output=" + output + ", description=" + description
				+ ", location=" + location.toString() + "]";
	}

	
}
