package es.ujaen.service;


import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;
import java.util.TimeZone;

import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import es.ujaen.Exceptions.DataException;
import es.ujaen.Exceptions.NodeException;
import es.ujaen.Exceptions.UserException;
import es.ujaen.entity.Data;
import es.ujaen.entity.DataInfo;
import es.ujaen.entity.Node;
import es.ujaen.entity.NodeInfo;
import es.ujaen.entity.User;
import es.ujaen.entity.UserInfo;
import es.ujaen.repository.DataRepository;
import es.ujaen.repository.NodeRepository;
import es.ujaen.repository.UserRepository;
import es.ujaen.securingweb.CustomAccessDeniedHandler;

@Service
public class DataService{
	
	private static final Logger logger = LoggerFactory.getLogger(DataService.class);

	
	@Autowired
	DataRepository dataRepository;
	
	@Autowired
	NodeService nodeService;
	
	public List<DataInfo> getByNodeId(long nodeId) throws NodeException, UserException{
		List<DataInfo> dataInfos = new ArrayList<>();
		
		
		NodeInfo nodeInfo = nodeService.getNodeInfoById(nodeId);
		
		List<Data> dataList = dataRepository.findByNodeId(nodeId);
		
		Iterator<Data> dataIterator = dataList.iterator();
		
		while(dataIterator.hasNext()) {
			DataInfo d = new DataInfo(dataIterator.next(), nodeInfo);
			dataInfos.add(d);
		}
		
		return dataInfos;
	}
	
	public List<DataInfo> getByNode(NodeInfo node){
		List<DataInfo> dataInfos = new ArrayList<>();
		
		List<Data> dataList = dataRepository.findByNodeId(node.getId());
		
		Iterator<Data> dataIterator = dataList.iterator();
		
		Calendar now = Calendar.getInstance();
		TimeZone tz = now.getTimeZone();
		
		while(dataIterator.hasNext()) {
			DataInfo d = new DataInfo(dataIterator.next(), node);
			d.setTimeSample(new Date(d.getTimeSample().getTime()-tz.getOffset(now.getTimeInMillis())+node.getLocation().getTimeZoneOffset()));	//resto zona horaria del servidor y sumo la del nodo
			dataInfos.add(d);
		}
		
		return dataInfos;
	}
	
	
	public List<DataInfo> getByNodeInfoOrderByTime(NodeInfo node){
		List<DataInfo> dataInfos = new ArrayList<>();
		
		List<Data> dataList = dataRepository.findByNodeInfoOrderByTime(node.getId());
		
		Iterator<Data> dataIterator = dataList.iterator();
		
		Calendar now = Calendar.getInstance();
		TimeZone tz = now.getTimeZone();
		
		while(dataIterator.hasNext()) {
			DataInfo d = new DataInfo(dataIterator.next(), node);
			d.setTimeSample(new Date(d.getTimeSample().getTime()-tz.getOffset(now.getTimeInMillis())+node.getLocation().getTimeZoneOffset()));	//resto zona horaria del servidor y sumo la del nodo
			dataInfos.add(d);
		}
		
		return dataInfos;
	}
	
	//añade con respecto a la versión anterior un parámetro para elegir que datos se devuelven en lugar de todos
	//es un valor de un byte cuyos bits indican que tipos de datos devolver
	public List<DataInfo> getByNodeInfoOrderByTime(NodeInfo node, int searchValues){
		List<DataInfo> dataInfos = new ArrayList<>();
		
		String binary = Integer.toBinaryString(searchValues);
				
		
		List<Data> dataList = dataRepository.findByNodeInfoOrderByTime(node.getId());
		
		Iterator<Data> dataIterator = dataList.iterator();
		
		Calendar now = Calendar.getInstance();
		TimeZone tz = now.getTimeZone();
		
		while(dataIterator.hasNext()) {
			DataInfo d = new DataInfo(dataIterator.next(), node);
			d.setTimeSample(new Date(d.getTimeSample().getTime()-tz.getOffset(now.getTimeInMillis())+node.getLocation().getTimeZoneOffset()));	//resto zona horaria del servidor y sumo la del nodo
			//si se busca con 0 (nada seleccionado) se devuelve todo		
			if (searchValues > 0) {
				if(binary.length() <= 0 || binary.charAt(binary.length()-1) != '1') {
					d.setCurrentBattery(0);
					d.setCurrentLoad(0);
					d.setCurrentPhotovoltaic(0);
				}
				if(binary.length() <= 1 || binary.charAt(binary.length()-2) != '1') {
					d.setVoltageBattery(0);
					d.setVoltageLoad(0);
					d.setVoltagePhotovoltaic(0);
				}
				if(binary.length() <= 2 || binary.charAt(binary.length()-3) != '1') {
					d.setTemperature(0);
					d.setTemperatureBattery(0);
					d.setTemperaturePhotovoltaic(0);
				}
				if(binary.length() <= 3 || binary.charAt(binary.length()-4) != '1') {
					d.setHumidity(0);
				}
				if(binary.length() <= 4 || binary.charAt(binary.length()-5) != '1') {
					d.setIrradiance(0);
				}
				if(binary.length() <= 5 || binary.charAt(binary.length()-6) != '1') {
					d.setPressure(0);
				}
				if(binary.length() <= 6 || binary.charAt(binary.length()-7) != '1') {
					d.setWind(0);
				}
				if(binary.length() <= 7 || binary.charAt(binary.length()-8) != '1') {
					d.setRainfall(0);
				}		
			}
			dataInfos.add(d);
		}
		
		return dataInfos;
	}
	
	
	//añade con respecto a la versión anterior un parámetro para elegir que datos se devuelven en lugar de todos
		//es un valor de un byte cuyos bits indican que tipos de datos devolver
		public List<DataInfo> getByNodeInfoOrderByTime(NodeInfo node, int searchValues, long unixtime_from, long unixtime_to){
			List<DataInfo> dataInfos = new ArrayList<>();
			
			String binary = Integer.toBinaryString(searchValues);
					
			List<Data> dataList = dataRepository.findByNodeInfoOrderByTime(node.getId(), unixtime_from, unixtime_to);
			
			Iterator<Data> dataIterator = dataList.iterator();
			
			Calendar now = Calendar.getInstance();
			TimeZone tz = now.getTimeZone();
			
			while(dataIterator.hasNext()) {
				DataInfo d = new DataInfo(dataIterator.next(), node);
				d.setTimeSample(new Date(d.getTimeSample().getTime()-tz.getOffset(now.getTimeInMillis())+node.getLocation().getTimeZoneOffset()));	//resto zona horaria del servidor y sumo la del nodo
				//si se busca con 0 (nada seleccionado) se devuelve todo		
				if (searchValues > 0) {
					if(binary.length() <= 0 || binary.charAt(binary.length()-1) != '1') {
						d.setCurrentBattery(0);
						d.setCurrentLoad(0);
						d.setCurrentPhotovoltaic(0);
					}
					if(binary.length() <= 1 || binary.charAt(binary.length()-2) != '1') {
						d.setVoltageBattery(0);
						d.setVoltageLoad(0);
						d.setVoltagePhotovoltaic(0);
					}
					if(binary.length() <= 2 || binary.charAt(binary.length()-3) != '1') {
						d.setTemperature(0);
						d.setTemperatureBattery(0);
						d.setTemperaturePhotovoltaic(0);
					}
					if(binary.length() <= 3 || binary.charAt(binary.length()-4) != '1') {
						d.setHumidity(0);
					}
					if(binary.length() <= 4 || binary.charAt(binary.length()-5) != '1') {
						d.setIrradiance(0);
					}
					if(binary.length() <= 5 || binary.charAt(binary.length()-6) != '1') {
						d.setPressure(0);
					}
					if(binary.length() <= 6 || binary.charAt(binary.length()-7) != '1') {
						d.setWind(0);
					}
					if(binary.length() <= 7 || binary.charAt(binary.length()-8) != '1') {
						d.setRainfall(0);
					}
					if(binary.length() <= 8 || binary.charAt(binary.length()-9) != '1') {
						d.setOutput(false);
					}	
				}
				dataInfos.add(d);
			}
			
			return dataInfos;
		}
	
	
	
	public Data findByHash(String hash) throws DataException{
		Optional<Data> optional = dataRepository.findByHash(hash);
		
		Data data = optional.orElseThrow(() -> new DataException(DataException.NO_HASH_FOUND));
		
		return data;
	}
	
	/*
	 * dado un hash comprueba si ya se ha añadido a la base de datos
	 */
	public boolean existsByHash(String hash){
		Optional<Data> optional = dataRepository.findByHash(hash);
		
		return optional.isPresent();
	}
	
	/*
	 * dado un hash comprueba si ya se ha añadido a la base de datos
	 */
	public boolean existsByHash(DataInfo dataInfo){
		Optional<Data> optional = dataRepository.findByHash(dataInfo.getJsonHash());
		
		return optional.isPresent();
	}
	
	
	public void save(DataInfo dataInfo) throws DataException {
		Data data = new Data(dataInfo);
		
		if(existsByHash(dataInfo.getJsonHash()) == false) {
			dataRepository.save(data);
		} else {
			throw new DataException(DataException.HASH_FOUND);
		}
		
	}
	
	
	/* dado un nodeInfo devuelve todos los datos para exportarlos como excel */
	public ByteArrayInputStream exportAllData(NodeInfo nodeInfo, long unixtime_from, long unixtime_to) throws IOException {
		
		ArrayList<String> ar = new ArrayList<>();
		
		//The excel header elements
		ar.add("id"); ar.add("timeSample"); ar.add("timeUpload"); ar.add("currentLoad"); ar.add("currentBattery"); ar.add("currentPhotovoltaic"); ar.add("voltageLoad");
		ar.add("voltageBattery"); ar.add("voltagePhotovoltaic"); ar.add("temperatureBattery"); ar.add("temperaturePhotovoltaic"); ar.add("output");
		
		if(nodeInfo.getFunction() == Node.FUNCTION_MASTER) {
			ar.add("irradiance"); ar.add("rainfall"); ar.add("wind");
			ar.add("humidity"); ar.add("temperature"); ar.add("pressure");
		}
		
		//creating workbook
		Workbook workbook = new HSSFWorkbook();
		
		ByteArrayOutputStream aos = new ByteArrayOutputStream();
		
		
		Sheet sheet = workbook.createSheet("Data");
		Row row = sheet.createRow(0);
		
		//add header
		for(int i = 0; i < ar.size(); i++) {
			Cell cell = row.createCell(i);
			cell.setCellValue(ar.get(i));
		}
		
		//getting datainfos
		List<DataInfo> dataInfos = getByNodeInfoOrderByTime(nodeInfo, 511, unixtime_from, unixtime_to);
		
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		
		//putting data
		int rowNum = 1;
		for(DataInfo dataInfo : dataInfos) {
			row = sheet.createRow(rowNum);
			
			row.createCell(0).setCellValue(dataInfo.getId());
			row.createCell(1).setCellValue(sdf.format(dataInfo.getTimeSample()));
			row.createCell(2).setCellValue(sdf.format(dataInfo.getTimeUpload()));
			row.createCell(3).setCellValue(dataInfo.getCurrentLoad());
			row.createCell(4).setCellValue(dataInfo.getCurrentBattery());
			row.createCell(5).setCellValue(dataInfo.getCurrentPhotovoltaic());
			row.createCell(6).setCellValue(dataInfo.getVoltageLoad());
			row.createCell(7).setCellValue(dataInfo.getVoltageBattery());
			row.createCell(8).setCellValue(dataInfo.getVoltagePhotovoltaic());
			row.createCell(9).setCellValue(dataInfo.getTemperatureBattery());
			row.createCell(10).setCellValue(dataInfo.getTemperaturePhotovoltaic());
			row.createCell(11).setCellValue(dataInfo.isOutput());
			
			if(nodeInfo.getFunction() == Node.FUNCTION_MASTER) {
				row.createCell(12).setCellValue(dataInfo.getIrradiance());
				row.createCell(13).setCellValue(dataInfo.getRainfall());
				row.createCell(14).setCellValue(dataInfo.getWind());
				row.createCell(15).setCellValue(dataInfo.getHumidity());
				row.createCell(16).setCellValue(dataInfo.getTemperature());
				row.createCell(17).setCellValue(dataInfo.getPressure());
			}
			
			rowNum++;
		}
		
		workbook.write(aos);
		workbook.close();
		
		return new ByteArrayInputStream(aos.toByteArray());
		
	}
	
}
