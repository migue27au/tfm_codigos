package es.ujaen.DTO;

import java.util.Date;

import org.json.simple.JSONObject;
import org.json.simple.JSONArray;
import org.json.simple.parser.ParseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.json.simple.parser.JSONParser;

import es.ujaen.entity.NodeInfo;

public class NodeInfoDTO {
	
	private static final Logger logger = LoggerFactory.getLogger(NodeInfoDTO.class);

	
	private String nodeName;
	private long msb;
	private long lsb;
	private int sampleRate = 0;
	private boolean output = false;
		
	
	public NodeInfoDTO() {
		super();
	}

	
	public NodeInfoDTO(NodeInfo nodeInfo) {
		super();
		this.nodeName = nodeInfo.getNodeName();
		this.msb = Long.decode("0x" + nodeInfo.getAddressMsb());
		this.lsb = Long.decode("0x" + nodeInfo.getAddressLsb());
		this.sampleRate = nodeInfo.getSampleRate();
		this.output = nodeInfo.isOutput();
	}
	
	public NodeInfoDTO(JSONObject json) {
		super();
		this.nodeName = (String) json.get("n");
		if(json.containsKey("m"))
			this.msb = Long.parseLong((String) json.get("m"));
		if(json.containsKey("l"))
			this.lsb = Long.parseLong((String) json.get("l"));
		if(json.containsKey("s"))
			this.sampleRate = Integer.parseInt((String) json.get("s"));
		if(json.containsKey("o")) {
			String outputValue = (String)json.get("o");
			if(outputValue.equals("1")) {
				this.output = true;
			} else {
				this.output = false;
			}
		}
	}
	
	public NodeInfoDTO(String nodeName, long msb, long lsb, int sampleRate, boolean output) {
		super();
		this.nodeName = nodeName;
		this.msb = msb;
		this.lsb = lsb;
		this.sampleRate = sampleRate;
		this.output = output;
	}


	public String getNodeName() {
		return nodeName;
	}


	public void setNodeName(String nodeName) {
		this.nodeName = nodeName;
	}


	public long getMsb() {
		return msb;
	}


	public void setMsb(long msb) {
		this.msb = msb;
	}


	public long getLsb() {
		return lsb;
	}


	public void setLsb(long lsb) {
		this.lsb = lsb;
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


	@Override
	public String toString() {
		return "NodeInfoDTO [nodeName=" + nodeName + ", msb=" + msb + ", lsb=" + lsb + ", sampleRate=" + sampleRate
				+ ", output=" + output + "]";
	}
	
	public String toSimpleJSONString() {
		JSONObject json = new JSONObject();
		json.put("n", this.nodeName);
		json.put("s", this.sampleRate);
		json.put("o", this.output);		
		
		return json.toJSONString();
	}
	
	public JSONObject toSimpleJSONObject() {
		JSONObject json = new JSONObject();
		json.put("n", this.nodeName);
		json.put("s", this.sampleRate);
		json.put("o", this.output);		
		
		return json;
	}
	
}
