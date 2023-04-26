package es.ujaen.entity;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import es.ujaen.DTO.UploadDataDTO;

@Entity
@Table(name = "data")
public class Data {
	
	private static final Logger logger = LoggerFactory.getLogger(Data.class);

	
	@Id
	@GeneratedValue(strategy = GenerationType.AUTO)
	@Column(name = "id", unique=true)
	long id;
	
	@Column(name = "nodeId")
	long nodeId;
	
	@Column(name = "timeSample")
	long timeSample;
	
	@Column(name = "timeUpload")
	long timeUpload;
	
	@Column(name = "currentLoad")
	float currentLoad;

	@Column(name = "currentBattery")
	float currentBattery;

	@Column(name = "currentPhotovoltaic")
	float currentPhotovoltaic;

	@Column(name = "voltageLoad")
	float voltageLoad;

	@Column(name = "voltageBattery")
	float voltageBattery;

	@Column(name = "voltagePhotovoltaic")
	float voltagePhotovoltaic;

	@Column(name = "irradiance")
	float irradiance;

	@Column(name = "rainfall")
	float rainfall;

	@Column(name = "wind")
	float wind;

	@Column(name = "humidity")
	float humidity;

	@Column(name = "temperature")
	float temperature;
	
	@Column(name = "pressure")
	float pressure;

	@Column(name = "temperatureBattery")
	float temperatureBattery;

	@Column(name = "temperaturePhotovoltaic")
	float temperaturePhotovoltaic;
	
	@Column(name = "output")
	boolean output;

	@Column(name = "hash")
	String hash;

		
	public Data() {
		super();
	}

	
	public Data(long id, long nodeId, long timeSample, long timeUpload, float currentLoad, float currentBattery,
			float currentPhotovoltaic, float voltageLoad, float voltageBattery, float voltagePhotovoltaic,
			float irradiance, float rainfall, float wind, float humidity, float temperature, float pressure, float temperatureBattery,
			float temperaturePhotovoltaic, boolean output, String hash) {
		super();
		this.id = id;
		this.nodeId = nodeId;
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
		this.hash = hash;
	}


	public Data(DataInfo dataInfo) {
		super();
		this.id = dataInfo.getId();
		this.nodeId = dataInfo.getNodeInfo().getId();
		this.timeSample = dataInfo.getTimeSample().getTime();
		this.timeUpload = dataInfo.getTimeUpload().getTime();
		this.currentLoad = dataInfo.getCurrentLoad();
		this.currentBattery = dataInfo.getCurrentBattery();
		this.currentPhotovoltaic = dataInfo.getCurrentPhotovoltaic();
		this.voltageLoad = dataInfo.getVoltageLoad();
		this.voltageBattery = dataInfo.getVoltageBattery();
		this.voltagePhotovoltaic = dataInfo.getVoltagePhotovoltaic();
		this.irradiance = dataInfo.getIrradiance();
		this.rainfall = dataInfo.getRainfall();
		this.wind = dataInfo.getWind();
		this.humidity = dataInfo.getHumidity();
		this.temperature = dataInfo.getTemperature();
		this.pressure = dataInfo.getPressure();
		this.temperatureBattery = dataInfo.getTemperatureBattery();
		this.temperaturePhotovoltaic = dataInfo.getTemperaturePhotovoltaic();
		this.output = dataInfo.isOutput();
		this.hash = dataInfo.getJsonHash();
	}
	

	public long getId() {
		return id;
	}



	public void setId(long id) {
		this.id = id;
	}



	public long getNodeId() {
		return nodeId;
	}



	public void setNodeId(long nodeId) {
		this.nodeId = nodeId;
	}



	public long getTimeSample() {
		return timeSample;
	}



	public void setTimeSample(long timeSample) {
		this.timeSample = timeSample;
	}



	public long getTimeUpload() {
		return timeUpload;
	}



	public void setTimeUpload(long timeUpload) {
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



	public String getHash() {
		return hash;
	}



	public void setHash(String hash) {
		this.hash = hash;
	}
	
	
	

}
