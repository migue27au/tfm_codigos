package es.ujaen.entity;

import java.util.Date;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import es.ujaen.DTO.PullDataDTO;
import es.ujaen.DTO.UploadDataDTO;

public class DataInfo {
	
	private static final Logger logger = LoggerFactory.getLogger(DataInfo.class);

	
	long id;
	NodeInfo nodeInfo;
	Date timeSample;
	Date timeUpload;
	float currentLoad;
	float currentBattery;
	float currentPhotovoltaic;
	float voltageLoad;
	float voltageBattery;
	float voltagePhotovoltaic;
	float irradiance;
	float rainfall;
	float wind;
	float humidity;
	float temperature;
	float pressure;
	float temperatureBattery;
	float temperaturePhotovoltaic;
	boolean output;
	String jsonHash;
		
	public DataInfo() {
		super();
	}
	
	

	public DataInfo(long id, NodeInfo nodeInfo, Date timeSample, Date timeUpload, float currentLoad,
			float currentBattery, float currentPhotovoltaic, float voltageLoad, float voltageBattery,
			float voltagePhotovoltaic, float irradiance, float rainfall, float wind, float humidity, float temperature,
			float pressure, float temperatureBattery, float temperaturePhotovoltaic, boolean output, String jsonHash) {
		super();
		this.id = id;
		this.nodeInfo = nodeInfo;
		this.timeSample = timeSample;
		this.timeUpload = timeUpload;
		this.currentLoad = currentLoad;
		this.currentBattery = currentBattery;
		this.currentPhotovoltaic = currentPhotovoltaic;
		this.voltageLoad = voltageLoad;
		this.voltageBattery = voltageBattery;
		this.voltagePhotovoltaic = voltagePhotovoltaic;
		this.irradiance = irradiance;
		this.rainfall = rainfall;
		this.wind = wind;
		this.humidity = humidity;
		this.temperature = temperature;
		this.pressure = pressure;
		this.temperatureBattery = temperatureBattery;
		this.temperaturePhotovoltaic = temperaturePhotovoltaic;
		this.output = output;
		this.jsonHash = jsonHash;
	}

			

	public DataInfo(PullDataDTO pullData, NodeInfo nodeInfo) {
		super();
		this.nodeInfo = nodeInfo;
		this.timeSample = new Date(pullData.getTime()*1000);
		this.timeUpload = new Date(System.currentTimeMillis());
		this.currentLoad = pullData.getCl();
		this.currentBattery = pullData.getCb();
		this.currentPhotovoltaic = pullData.getCp();
		this.voltageLoad = pullData.getVl();
		this.voltageBattery = pullData.getVb();
		this.voltagePhotovoltaic = pullData.getVp();
		this.irradiance = pullData.getI();
		this.rainfall = pullData.getR();
		this.wind = pullData.getW();
		this.humidity = pullData.getH();
		this.temperature = pullData.getT();
		this.pressure = pullData.getP();
		this.temperatureBattery = pullData.getTb();
		this.temperaturePhotovoltaic = pullData.getTp();
		this.output = pullData.isOutput();
		this.jsonHash = pullData.getVer();
	}

	public DataInfo(Data data, NodeInfo nodeInfo) {
		super();
		this.id = data.getId();
		this.nodeInfo = nodeInfo;
		this.timeSample = new Date(data.getTimeSample());
		this.timeUpload = new Date(data.getTimeUpload());
		this.currentLoad = data.getCurrentLoad();
		this.currentBattery = data.getCurrentBattery();
		this.currentPhotovoltaic = data.getCurrentPhotovoltaic();
		this.voltageLoad = data.getVoltageLoad();
		this.voltageBattery = data.getVoltageBattery();
		this.voltagePhotovoltaic = data.getVoltagePhotovoltaic();
		this.irradiance = data.getIrradiance();
		this.rainfall = data.getRainfall();
		this.wind = data.getWind();
		this.humidity = data.getHumidity();
		this.temperature = data.getTemperature();
		this.pressure = data.getPressure();
		this.temperatureBattery = data.getTemperatureBattery();
		this.temperaturePhotovoltaic = data.getTemperaturePhotovoltaic();
		this.output = data.isOutput();
		this.jsonHash = data.getHash();
	}


	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public NodeInfo getNodeInfo() {
		return nodeInfo;
	}

	public void setNodeInfo(NodeInfo nodeInfo) {
		this.nodeInfo = nodeInfo;
	}

	public Date getTimeSample() {
		return timeSample;
	}

	public void setTimeSample(Date timeSample) {
		this.timeSample = timeSample;
	}

	public Date getTimeUpload() {
		return timeUpload;
	}

	public void setTimeUpload(Date timeUpload) {
		this.timeUpload = timeUpload;
	}

	public float getCurrentLoad() {
		return currentLoad;
	}

	public void setCurrentLoad(float currentLoad) {
		this.currentLoad = currentLoad;
	}

	public float getCurrentBattery() {
		return currentBattery;
	}

	public void setCurrentBattery(float currentBattery) {
		this.currentBattery = currentBattery;
	}

	public float getCurrentPhotovoltaic() {
		return currentPhotovoltaic;
	}

	public void setCurrentPhotovoltaic(float currentPhotovoltaic) {
		this.currentPhotovoltaic = currentPhotovoltaic;
	}

	public float getVoltageLoad() {
		return voltageLoad;
	}

	public void setVoltageLoad(float voltageLoad) {
		this.voltageLoad = voltageLoad;
	}

	public float getVoltageBattery() {
		return voltageBattery;
	}

	public void setVoltageBattery(float voltageBattery) {
		this.voltageBattery = voltageBattery;
	}

	public float getVoltagePhotovoltaic() {
		return voltagePhotovoltaic;
	}

	public void setVoltagePhotovoltaic(float voltagePhotovoltaic) {
		this.voltagePhotovoltaic = voltagePhotovoltaic;
	}

	public float getIrradiance() {
		return irradiance;
	}

	public void setIrradiance(float irradiance) {
		this.irradiance = irradiance;
	}

	public float getRainfall() {
		return rainfall;
	}

	public void setRainfall(float rainfall) {
		this.rainfall = rainfall;
	}

	public float getWind() {
		return wind;
	}

	public void setWind(float wind) {
		this.wind = wind;
	}

	public float getHumidity() {
		return humidity;
	}

	public void setHumidity(float humidity) {
		this.humidity = humidity;
	}

	public float getTemperature() {
		return temperature;
	}

	public void setTemperature(float temperature) {
		this.temperature = temperature;
	}

	
	public float getPressure() {
		return pressure;
	}



	public void setPressure(float pressure) {
		this.pressure = pressure;
	}



	public float getTemperatureBattery() {
		return temperatureBattery;
	}

	public void setTemperatureBattery(float temperatureBattery) {
		this.temperatureBattery = temperatureBattery;
	}

	public float getTemperaturePhotovoltaic() {
		return temperaturePhotovoltaic;
	}

	public void setTemperaturePhotovoltaic(float temperaturePhotovoltaic) {
		this.temperaturePhotovoltaic = temperaturePhotovoltaic;
	}


	public boolean isOutput() {
		return output;
	}

	public void setOutput(boolean output) {
		this.output = output;
	}


	public String getJsonHash() {
		return jsonHash;
	}

	public void setJsonHash(String jsonHash) {
		this.jsonHash = jsonHash;
	}


	@Override
	public String toString() {
		return "DataInfo [id=" + id + ", nodeInfo=" + nodeInfo + ", timeSample=" + timeSample + ", timeUpload="
				+ timeUpload + ", currentLoad=" + currentLoad + ", currentBattery=" + currentBattery
				+ ", currentPhotovoltaic=" + currentPhotovoltaic + ", voltageLoad=" + voltageLoad + ", voltageBattery="
				+ voltageBattery + ", voltagePhotovoltaic=" + voltagePhotovoltaic + ", irradiance=" + irradiance
				+ ", rainfall=" + rainfall + ", wind=" + wind + ", humidity=" + humidity + ", temperature="
				+ temperature + ", pressure=" + pressure + ", temperatureBattery=" + temperatureBattery
				+ ", temperaturePhotovoltaic=" + temperaturePhotovoltaic + ", output=" + output + ", jsonHash="
				+ jsonHash + "]";
	}		
}
