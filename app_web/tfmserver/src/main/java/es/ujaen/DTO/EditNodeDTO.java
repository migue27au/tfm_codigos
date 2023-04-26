package es.ujaen.DTO;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import es.ujaen.entity.NodeInfo;
import es.ujaen.utils.SanitizeUtil;

public class EditNodeDTO {

	private static final Logger logger = LoggerFactory.getLogger(EditNodeDTO.class);

	
	private long nodeId;
	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9_ ()\u00f1\u00d1]{1,30}$", message = "Error in EditNodeDTO validation")
	String alias;
	
	@Min(20)
	private int sampleRate;
	
	private boolean output;
	
	private double longitude;
	private double latitude;
	
	@Min(-11)
	@Max(12)
	private int timeZoneFirst;
	
	@Min(0)
	@Max(59)
	private int timeZoneSecond;
	
	private String description;
	
	
	public EditNodeDTO(long nodeId, String alias, @Min(20) int sampleRate, boolean output, double longitude,
			double latitude, @Min(-11) @Max(12) int timeZoneFirst, @Min(0) @Max(59) int timeZoneSecond,
			String description) {
		super();
		this.nodeId = nodeId;
		this.alias = alias;
		this.sampleRate = sampleRate;
		this.output = output;
		this.longitude = longitude;
		this.latitude = latitude;
		this.timeZoneFirst = timeZoneFirst;
		this.timeZoneSecond = timeZoneSecond;
		//this.description = SanitizeUtil.toHTMLEncode(description);
		this.description = description;
	}


	public EditNodeDTO(NodeInfo nodeInfo) {
		super();
		if(nodeInfo != null && nodeInfo.getLocation() != null) {
			this.alias = nodeInfo.getAlias();
			this.nodeId = nodeInfo.getId();
			this.sampleRate = nodeInfo.getSampleRate();
			this.output = nodeInfo.isOutput();
			this.longitude = nodeInfo.getLocation().getLongitude();
			this.latitude = nodeInfo.getLocation().getLatitude();
			if(nodeInfo.getLocation().getTimeZone() != null && nodeInfo.getLocation().getTimeZone().contains(":")) {
				this.timeZoneFirst = Integer.parseInt(nodeInfo.getLocation().getTimeZone().split(":")[0]);
				this.timeZoneSecond = Integer.parseInt(nodeInfo.getLocation().getTimeZone().split(":")[1]);
			} else {
				this.timeZoneFirst = 0;
				this.timeZoneSecond = 0;
			}
			this.description = nodeInfo.getDescription();
		}
	}
	
	
	public EditNodeDTO() {
		super();
	}
	
	
	public String getAlias() {
		return alias;
	}

	public void setAlias(String alias) {
		this.alias = alias;
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

	public double getLongitude() {
		return longitude;
	}
	public void setLongitude(double longitude) {
		this.longitude = longitude;
	}
	public double getLatitude() {
		return latitude;
	}
	public void setLatitude(double latitude) {
		this.latitude = latitude;
	}

	public long getNodeId() {
		return nodeId;
	}

	public void setNodeId(long nodeId) {
		this.nodeId = nodeId;
	}

	public int getTimeZoneFirst() {
		return timeZoneFirst;
	}


	public void setTimeZoneFirst(int timeZoneFirst) {
		this.timeZoneFirst = timeZoneFirst;
	}


	public int getTimeZoneSecond() {
		return timeZoneSecond;
	}


	public void setTimeZoneSecond(int timeZoneSecond) {
		this.timeZoneSecond = timeZoneSecond;
	}


	public String getDescription() {
		return description;
	}

	public void setDescription(String description) {
		this.description = description;
	}


	@Override
	public String toString() {
		return "EditNodeDTO [nodeId=" + nodeId + ", alias=" + alias + ", sampleRate=" + sampleRate + ", output="
				+ output + ", longitude=" + longitude + ", latitude=" + latitude + ", timeZoneFirst=" + timeZoneFirst
				+ ", timeZoneSecond=" + timeZoneSecond + ", description=" + description + "]";
	}

	
	
}
