package es.ujaen.DTO;

import java.util.Date;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

import org.json.simple.JSONObject;
import org.json.simple.JSONArray;
import org.json.simple.parser.ParseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.json.simple.parser.JSONParser;

import es.ujaen.entity.NodeInfo;

public class PullDataDTO {
	
	private static final Logger logger = LoggerFactory.getLogger(PullDataDTO.class);

	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9_]{8,8}$", message = "Error en la validación")
	private String name;
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9]{1,64}$", message = "Error in PullData validation")
	private String rand;
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9]{64,64}$", message = "Error in PullData validation")
	private String hash;
	
	private long time;
	private float cl; //currentLoad;
	private float cb; //currentBattery;
	private float cp; //currentPhotovoltaic;
	private float vl; //voltageLoad;
	private float vb; //voltageBattery;
	private float vp; //voltagePhotovoltaic;
	private float i; //irradiance;
	private float r; //rainfall;
	private float w; //wind;
	private float h; //humidity;
	private float t; //temperature;
	private float p; //pressure
	private float tb; //temperatureBattery;
	private float tp; //temperaturePhotovoltaic;
	private boolean output; //output1;
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9]{64,64}$", message = "Error in PullData validation")
	private String ver;
	
	
	
	public PullDataDTO() {
		super();
	}

	
	//TODO: validar todo esto
	public PullDataDTO(JSONObject json) {
		super();
		this.name = (String) json.get("name");
		this.rand = (String) json.get("rand");
		this.hash = (String) json.get("hash");
		this.time = Long.parseLong((String)json.get("time"));
		
		this.cl = Float.parseFloat((String)json.get("cl"));
		this.cb = Float.parseFloat((String)json.get("cb"));
		this.cp = Float.parseFloat((String)json.get("cp"));
		this.vl = Float.parseFloat((String)json.get("vl"));
		this.vb = Float.parseFloat((String)json.get("vb"));
		this.vp = Float.parseFloat((String)json.get("vp"));
		this.i = Float.parseFloat((String)json.get("i"));
		this.r = Float.parseFloat((String)json.get("r"));
		this.w = Float.parseFloat((String)json.get("w"));
		this.h = Float.parseFloat((String)json.get("h"));
		this.t = Float.parseFloat((String)json.get("t"));
		this.p = Float.parseFloat((String)json.get("p"));
		this.tb = Float.parseFloat((String)json.get("tb"));
		this.tp = Float.parseFloat((String)json.get("tp"));
		this.output = ((String)json.get("out")).matches("1");				
		
		this.ver = (String) json.get("ver");
	}
	

	public PullDataDTO(String name, String rand, String hash, long time, float cl, float cb, float cp, float vl,
			float vb, float vp, float i, float r, float w, float h, float t, float p, float tb, float tp, boolean output,
			String ver) {
		super();
		this.name = name;
		this.rand = rand;
		this.hash = hash;
		this.time = time;
		this.cl = cl;
		this.cb = cb;
		this.cp = cp;
		this.vl = vl;
		this.vb = vb;
		this.vp = vp;
		this.i = i;
		this.r = r;
		this.w = w;
		this.h = h;
		this.t = t;
		this.p = p;
		this.tb = tb;
		this.tp = tp;
		this.output = output;
		this.ver = ver;
	}



	public String getName() {
		return name;
	}



	public void setName(String name) {
		this.name = name;
	}



	public String getRand() {
		return rand;
	}



	public void setRand(String rand) {
		this.rand = rand;
	}



	public String getHash() {
		return hash;
	}



	public void setHash(String hash) {
		this.hash = hash;
	}



	public long getTime() {
		return time;
	}



	public void setTime(long time) {
		this.time = time;
	}



	public float getCl() {
		return cl;
	}



	public void setCl(float cl) {
		this.cl = cl;
	}



	public float getCb() {
		return cb;
	}



	public void setCb(float cb) {
		this.cb = cb;
	}



	public float getCp() {
		return cp;
	}



	public void setCp(float cp) {
		this.cp = cp;
	}



	public float getVl() {
		return vl;
	}



	public void setVl(float vl) {
		this.vl = vl;
	}



	public float getVb() {
		return vb;
	}



	public void setVb(float vb) {
		this.vb = vb;
	}



	public float getVp() {
		return vp;
	}



	public void setVp(float vp) {
		this.vp = vp;
	}



	public float getI() {
		return i;
	}



	public void setI(float i) {
		this.i = i;
	}



	public float getR() {
		return r;
	}



	public void setR(float r) {
		this.r = r;
	}



	public float getW() {
		return w;
	}



	public void setW(float w) {
		this.w = w;
	}



	public float getH() {
		return h;
	}



	public void setH(float h) {
		this.h = h;
	}



	public float getT() {
		return t;
	}



	public void setT(float t) {
		this.t = t;
	}



	public float getP() {
		return p;
	}


	public void setP(float p) {
		this.p = p;
	}


	public float getTb() {
		return tb;
	}



	public void setTb(float tb) {
		this.tb = tb;
	}



	public float getTp() {
		return tp;
	}



	public void setTp(float tp) {
		this.tp = tp;
	}



	public boolean isOutput() {
		return output;
	}



	public void setOutput(boolean output) {
		this.output = output;
	}



	public String getVer() {
		return ver;
	}



	public void setVer(String ver) {
		this.ver = ver;
	}


	@Override
	public String toString() {
		return "PullDataDTO [name=" + name + ", rand=" + rand + ", hash=" + hash + ", time=" + time + ", cl=" + cl
				+ ", cb=" + cb + ", cp=" + cp + ", vl=" + vl + ", vb=" + vb + ", vp=" + vp + ", i=" + i + ", r=" + r
				+ ", w=" + w + ", h=" + h + ", t=" + t + ", p=" + p + ", tb=" + tb + ", tp=" + tp + ", output=" + output
				+ ", ver=" + ver + "]";
	}
 
	
}
