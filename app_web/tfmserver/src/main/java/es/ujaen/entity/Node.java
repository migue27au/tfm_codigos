package es.ujaen.entity;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import es.ujaen.DTO.EditNodeDTO;
import es.ujaen.DTO.NodeInfoDTO;
import es.ujaen.Exceptions.NodeException;

@Entity
@Table(name = "node")
public class Node {

	private static final Logger logger = LoggerFactory.getLogger(Node.class);

	
	public static final String SHARED_WITH_SPLIT = ",";
	
	public static final int NO_FUNCTION = 0;
	public static final int FUNCTION_MASTER = 1;
	public static final int FUNCTION_ROUTER = 2;
	public static final int FUNCTION_SLAVE = 3;
	
	public static final String DEFAULT_TIMEZONE = "+00:00";
	
	@Id
	@GeneratedValue(strategy = GenerationType.AUTO)
	@Column(name = "id", unique=true)
	long id;
	
	@Column(name = "addressLsb")
	long addressLsb;
	
	@Column(name = "addressMsb")
	long addressMsb;
	
	@Column(name = "function")
	int function;
	
	@Column(name = "alias", length=50)
	String alias;
	
	@Column(name = "masterId")
	long masterId;
	
	@Column(name = "ownerId")
	long ownerId;
	
	@Column(name = "nodeName", length=50, unique=true)
	String nodeName;	//esto es utilizado para autenticar un nodo y para saber de quien és
	
	@Column(name = "nodePass", length=64)
	String nodePass;
	
	@Column(name = "sampleRate")
	int sampleRate;
	
	@Column(name = "output")
	boolean output;

	@Column(name = "sharedWith", length=512)
	String sharedWith;
	
	@Column(name = "latitude")
	double latitude;
	
	@Column(name = "longitude")
	double longitude;
	
	@Column(name = "description", length=512)
	String description;
	
	@Column(name = "timeZone", length=10)
	String timeZone;
	
	public Node() {
		super();
	}
	
	public Node(String nodeName, String nodePass, long ownerId) {
		super();
		this.nodeName = nodeName;
		this.nodePass = nodePass;
		this.ownerId = ownerId;
		this.timeZone = DEFAULT_TIMEZONE;
	}

	
	public Node(long id, long addressLsb, long addressMsb, int function, String alias, long masterId, long ownerId,
			String nodeName, String nodePass, int sampleRate, boolean output, String sharedWith,
			double latitude, double longitude, String description, String timeZone) {
		super();
		this.id = id;
		this.addressLsb = addressLsb;
		this.addressMsb = addressMsb;
		this.function = function;
		this.alias = alias;
		this.masterId = masterId;
		this.ownerId = ownerId;
		this.nodeName = nodeName;
		this.nodePass = nodePass;
		this.sampleRate = sampleRate;
		this.output = output;
		this.sharedWith = sharedWith;
		this.latitude = latitude;
		this.longitude = longitude;
		this.description = description;
		this.timeZone = timeZone;
	}


	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public long getAddressLsb() {
		return addressLsb;
	}

	public void setAddressLsb(long addressLsb) {
		this.addressLsb = addressLsb;
	}

	public long getAddressMsb() {
		return addressMsb;
	}

	public void setAddressMsb(long addressMsb) {
		this.addressMsb = addressMsb;
	}

	public int getFunction() {
		return function;
	}

	public void setFunction(int function) {
		this.function = function;
	}

	public String getAlias() {
		return alias;
	}

	public void setAlias(String alias) {
		this.alias = alias;
	}

	public long getMasterId() {
		return masterId;
	}

	public void setMasterId(long masterId) {
		this.masterId = masterId;
	}

	public long getOwnerId() {
		return ownerId;
	}

	public void setOwnerId(long ownerId) {
		this.ownerId = ownerId;
	}

	public String getNodeName() {
		return nodeName;
	}

	public void setNodeName(String nodeName) {
		this.nodeName = nodeName;
	}

	public String getNodePass() {
		return nodePass;
	}

	public void setNodePass(String nodePass) {
		this.nodePass = nodePass;
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

	public String getSharedWith() {
		return sharedWith;
	}

	public void setSharedWith(String sharedWith) {
		this.sharedWith = sharedWith;
	}
	
	public void setNewSharedWith(long userId) throws NodeException {
		if(isSharedWith(userId)) {
			throw new NodeException(NodeException.SPECIFIED_NODE_ALREADY_SHARED_WITH_USER);
		} else {
			this.sharedWith += userId + ",";
		}
	}
	
	public double getLatitude() {
		return latitude;
	}

	public void setLatitude(double latitude) {
		this.latitude = latitude;
	}

	public double getLongitude() {
		return longitude;
	}

	public void setLongitude(double longitude) {
		this.longitude = longitude;
	}
	
	public String getDescription() {
		return description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	public boolean isSharedWith(long userId) {
		return this.getSharedWith().contains("," + userId + "," );
	}
	
	public String getTimeZone() {
		return timeZone;
	}

	public void setTimeZone(String timeZone) {
		this.timeZone = timeZone;
	}


	public void updateByEditNodeDTO(EditNodeDTO editNode) {
		this.alias = editNode.getAlias();
		this.sampleRate = editNode.getSampleRate();
		this.latitude = editNode.getLatitude();
		this.longitude = editNode.getLongitude();
			
		String tz = "";
		if(editNode.getTimeZoneFirst() >= 0) {
			tz = "+";
			if(editNode.getTimeZoneFirst() < 10) {
				tz += "0";
			}
			tz += String.valueOf(editNode.getTimeZoneFirst());
		} else {
			tz = "-";
			if(editNode.getTimeZoneFirst()*-1 < 10) {
				tz += "0";
			}
			tz += String.valueOf(editNode.getTimeZoneFirst()*-1);
		}
		tz += ":";
		if(editNode.getTimeZoneSecond() < 10) {
			tz += "0";
		}
		tz += String.valueOf(editNode.getTimeZoneSecond());
		
		this.timeZone = tz;
		this.output = editNode.isOutput();
		this.description = editNode.getDescription();
	}
	
	public void updateParameters(NodeInfoDTO nodeInfoDTO) {
		this.addressLsb = nodeInfoDTO.getLsb();
		this.addressMsb = nodeInfoDTO.getMsb();
		this.output = nodeInfoDTO.isOutput();
		this.sampleRate = nodeInfoDTO.getSampleRate();
	}
}
