package es.ujaen.controller;

import java.io.IOException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.validation.Valid;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;

import org.json.simple.JSONObject;
import org.json.simple.JSONArray;
import org.json.simple.parser.ParseException;
import org.json.simple.parser.JSONParser;

import es.ujaen.DTO.NodeInfoDTO;
import es.ujaen.DTO.PullDataDTO;
import es.ujaen.DTO.UploadDataDTO;
import es.ujaen.Exceptions.DataException;
import es.ujaen.Exceptions.NodeException;
import es.ujaen.Exceptions.UserException;
import es.ujaen.Exceptions.UserNodeRelationsException;
import es.ujaen.entity.Data;
import es.ujaen.entity.DataInfo;
import es.ujaen.entity.NodeInfo;
import es.ujaen.entity.UserInfo;
import es.ujaen.entity.UserNodeRelations;
import es.ujaen.entity.UserNodeRelationsInfo;
import es.ujaen.service.DataService;
import es.ujaen.service.NodeService;
import es.ujaen.service.UserNodeRelationsService;
import es.ujaen.service.UserService;
import es.ujaen.utils.AESUtil;
import es.ujaen.utils.SHA256Util;
import es.ujaen.utils.SerializeUtil;

@RestController
@RequestMapping("/rest")
public class RestResource {
	
	@Autowired
	private UserNodeRelationsService userNodeRelationsService;
	
	@Autowired
	private DataService dataService;
	
	@Autowired
	private UserService userService;
	
	@Autowired
	private NodeService nodeService;
	
	//logger
	private static final Logger logger = LoggerFactory.getLogger(RestResource.class);
	
	
	//SUBIR LOS PULLDATA
	@PostMapping("/create")
	public ResponseEntity<String> create(@Valid @RequestBody UploadDataDTO uploadDataDTO) throws DataException, InvalidKeySpecException, ParseException, NodeException, UserException{
		
		logger.info("--------POST REST CREATE--------");
				
		ResponseEntity<String> httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
		
		try {
			UserNodeRelationsInfo userNodeRelation = userNodeRelationsService.authenticate(uploadDataDTO);	//Autorización
			logger.info(String.format("Node autorized: %s",userNodeRelation.toString()));
			//DATOS: {"authName":"prueba", "nonce":"EC2AB550245E8C2B71CE894E9BCD439088BF3C5D6335EF8AED3A5A7F", "hash":"439EF9573DB334A02D9ECB1120450301B22F1B6AC82560F7E9EC74EA0DEA91F9", "data":"{"authName":"prueba", "nonce":"EC2AB550245E8C2B71CE894E9BCD439088BF3C5D6335EF8AED3A5A7F", "hash":"439EF9573DB334A02D9ECB1120450301B22F1B6AC82560F7E9EC74EA0DEA91F9", "data":"XNEHUbUxqpMLWnr7hghaBWSRlTdbnXqY7LQUsnzhzCeeGkUu00t3h0/Atz2QtJ9O1VRwu57+CPGuU1mWcoUdcTUT4w13zD+QRHEAmX0TTc/LMlYBLbgorzpbwyP2jWXfYqomvoM9NIMkQ1VDhq/F14ggnYP8yPnakT32AhTBG+A="}"}
			
			//TODO: IDENTIFICAR NODOS QUE ENVIAN DATOS PARA LUEGO SABER A QUIEN HAY QUE ENVIARLE LOS DATOS DE TODOS LOS SLAVES
						
			try {
				UserInfo owner = userNodeRelation.getUserInfo();
				
				String plain = AESUtil.decryptInput(uploadDataDTO, userNodeRelation);
				
				Object obj = new JSONParser().parse(plain);
				JSONArray array = (JSONArray) obj;
				
				Iterator<JSONObject> iterator = array.iterator();
				while(iterator.hasNext()) {
					PullDataDTO pullData = new PullDataDTO(iterator.next());
					logger.info(String.format("Pulldata received: %s", pullData.toString()));
				
					
					try {
						NodeInfo node = nodeService.authenticate(pullData);		//autenticación
						logger.info(String.format("Node autenticated: %s", node.toString()));
						
						DataInfo dataInfo = new DataInfo(pullData, node);
						
						//añado el nodo a la lista de nodos de este userNodeRelation
						if(userNodeRelation.checkIfNodeIsInNodeList(node) == false) {
							logger.info(String.format("Node added to user_node_relations node_list"));
							userNodeRelation.getNodes().add(node);
						}
						
						userNodeRelationsService.save(userNodeRelation);
						if(dataService.existsByHash(dataInfo) == false) {
							dataService.save(dataInfo);
							logger.info(String.format("pulldata added to db"));
							httpResponse = new ResponseEntity<String>(HttpStatus.OK);
						} else {
							logger.info(String.format("pulldata exists added"));
							httpResponse = new ResponseEntity<String>(HttpStatus.CONFLICT);
						}
					} catch (NodeException e) {
						if(e.getType() == NodeException.NODE_NOT_FOUND) {
							logger.error(String.format("Node cannot be autenticated"));
							e.printStackTrace();
							httpResponse = new ResponseEntity<String>(HttpStatus.PARTIAL_CONTENT);	//puede que haya un fallo en un pulldata pero no en el resto
						} else {
							logger.error(String.format("Exception: %s", e.getMessage()));
							e.printStackTrace();
							httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
						}
					}
					
				}
				
				
			} catch (InvalidKeyException | NoSuchPaddingException | NoSuchAlgorithmException | InvalidAlgorithmParameterException | BadPaddingException | IllegalBlockSizeException | UserException e) {
				httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
				// TODO Auto-generated catch block
				logger.error(String.format("Exception: %s", e.getMessage()));
				e.printStackTrace();
			}
				
			
		} catch (UserNodeRelationsException e) {
			// TODO Auto-generated catch block
			logger.error(String.format("Node unautorized: %s", e.getMessage()));
			e.printStackTrace();
			httpResponse = new ResponseEntity<String>(HttpStatus.UNAUTHORIZED);
		}
		
				
		return httpResponse;
	}
	
	//ejemplo para probar la conexión a la app rest
	@GetMapping("/test")
	public ResponseEntity<String> test() throws DataException, InvalidKeySpecException, ParseException, NodeException, UserException{
		
		logger.info("--------GET REST TEST--------");
				
		ResponseEntity<String> httpResponse;
		Calendar now = Calendar.getInstance();
		long time = (now.getTimeInMillis())/1000;	//UTC+0
		httpResponse = new ResponseEntity<String>("Everithing looks ok. This is the time " + String.valueOf(time), HttpStatus.OK);
				
		return httpResponse;
	}
	
	@PostMapping("/time")
	public ResponseEntity<String> time(@Valid @RequestBody UploadDataDTO uploadDataDTO) throws DataException, InvalidKeySpecException, ParseException, NodeException, UserException{
		
		logger.info("--------POST REST TIME--------");
		
		ResponseEntity<String> httpResponse;
		
		try {
			UserNodeRelationsInfo userNodeRelation = userNodeRelationsService.authenticate(uploadDataDTO);	//Autenticación
			logger.info(String.format("Node autenticated: %s",userNodeRelation.toString()));
			
			Calendar now = Calendar.getInstance();
			long time = (now.getTimeInMillis())/1000;	//UTC+0
			
			logger.info(String.format("Time returned: %d", time));
			httpResponse = new ResponseEntity<String>(String.valueOf(time), HttpStatus.OK);
				
		} catch (UserNodeRelationsException e) {
			// TODO Auto-generated catch block
			logger.error(String.format("Node unautorized: %s", e.getMessage()));
			e.printStackTrace();
			httpResponse = new ResponseEntity<String>(HttpStatus.UNAUTHORIZED);
		}
		
				
		return httpResponse;
	}
	
	
	@PostMapping("/get")
	public ResponseEntity<String> get(@RequestBody UploadDataDTO uploadDataDTO) throws DataException, InvalidKeySpecException, ParseException, NodeException, UserException{
		
		logger.info("--------POST REST GET--------");
		
		ResponseEntity<String> httpResponse;
		
		logger.info(uploadDataDTO.toString());
		
		try {
			UserNodeRelationsInfo userNodeRelation = userNodeRelationsService.authenticate(uploadDataDTO);	//Autenticación
			logger.info(String.format("Node autenticated: %s",userNodeRelation.toString()));			
			
			
			try {
				UserInfo owner = userNodeRelation.getUserInfo();
				
				String plain = AESUtil.decryptInput(uploadDataDTO, userNodeRelation);
				logger.info("Plain text: " + plain);
				//TODO: responder con datos del authName solicitado, comprobar que se tienen datos de ese authname			
				//se recibe una lista JSON con los authName de los nodos. Obtengo los nodos de dichos authname
				
				Object obj = new JSONParser().parse(plain);	//creo JSON array
				JSONArray array = (JSONArray) obj;
				
				Iterator<JSONObject> iterator = array.iterator();
				
				List<NodeInfoDTO> nodeInfos = new ArrayList<>();
				
				if(array.size() >= 1) {
					while(iterator.hasNext()) {
						NodeInfoDTO nodeReceived = new NodeInfoDTO(iterator.next());
						NodeInfoDTO nodeInfoToSend = new NodeInfoDTO(nodeService.getNodeInfoByNodeName(nodeReceived.getNodeName()));
						
						nodeInfos.add(nodeInfoToSend);
						
						logger.info(String.format("Response: %s", nodeInfoToSend.toSimpleJSONString()));
						
					}
					
					String json = "[";
					for(NodeInfoDTO nodeInfo:nodeInfos) {
						json += nodeInfo.toSimpleJSONString() + ",";
					}
					
					json = json.substring(0, json.length()-1) + "]";
						
					httpResponse = new ResponseEntity<String>(json, HttpStatus.OK);
				} else if(array.size() > 20) {
					httpResponse = new ResponseEntity<String>(HttpStatus.PAYLOAD_TOO_LARGE);
				} else {
					httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
				}
				
				
				
			} catch (InvalidKeyException | NoSuchPaddingException | NoSuchAlgorithmException | InvalidAlgorithmParameterException | BadPaddingException | IllegalBlockSizeException e) {
				httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
				logger.error(String.format("Exception: %s", e.getMessage()));
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		} catch (UserNodeRelationsException e) {
			// TODO Auto-generated catch block
			logger.error(String.format("Node unautorized: %s", e.getMessage()));
			e.printStackTrace();
			
			httpResponse = new ResponseEntity<String>(HttpStatus.UNAUTHORIZED);
		}
		
				
		return httpResponse;
	}
	
	//info se envía cuando un nodo es conectado al servidor.
	@PostMapping("/info")
	public ResponseEntity<String> info(@Valid @RequestBody UploadDataDTO uploadDataDTO) throws DataException, InvalidKeySpecException, ParseException, UserException{
		
		logger.info("--------POST REST INFO--------");
		
		
		
		ResponseEntity<String> httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);;
		
		try {
			UserNodeRelationsInfo userNodeRelation = userNodeRelationsService.authenticate(uploadDataDTO);	//Autenticación
			logger.info(String.format("Node autenticated: %s",userNodeRelation.toString()));			
			
			try {
				UserInfo owner = userNodeRelation.getUserInfo();
				
				String plain = AESUtil.decryptInput(uploadDataDTO, userNodeRelation);
				
				logger.info(plain);
				
				Object obj = new JSONParser().parse(plain);	//creo JSON array
				JSONArray array = (JSONArray) obj;
									
				Iterator<JSONObject> iterator = array.iterator();
				
				while(iterator.hasNext()) {
					NodeInfoDTO nodeReceived = new NodeInfoDTO(iterator.next());	//creo nodeInfoDTO con el objeto JSON recibido
					try {
						NodeInfo node = nodeService.updateParametersByNodeInfoDTO(nodeReceived);	//actualizo los parámetros con los recibidos
						logger.info(String.format("Node: %s", node.toString()));
						
						//permitir cambiar de nodriza a un slave
						
						//compruebo si el nodo está añadido a algún nodes de userNodeRelations
						//si sí lo está
						if(userNodeRelationsService.checkIfExistsByNodeIdInNodes(node.getId())) {
							logger.info(String.format("Node exists in user_node_relations node_list"));
							//obtengo el userNodeRelations de ese nodo
							UserNodeRelationsInfo userNodeRelationsPrevious = userNodeRelationsService.getUserNodeRelationsInfoByNodeId(node.getId());
							//compruebo si son el mismo node remote previous, si no son el mismo quiere decir que está conectado a otro nodriza
							if(userNodeRelationsPrevious.getId() != userNodeRelation.getId()) {
								logger.info(String.format("Node have change master node. Removing from previous master..."));
								//entonces borro el node del antiguo
								userNodeRelationsService.removeNodeFromNodeList(userNodeRelationsPrevious, node.getId());
							}
							
						}
						//finalmente añado el nodo a nodes
						logger.info(String.format("Node added to user_node_relations node_list"));

						userNodeRelationsService.addNodeToNodeList(userNodeRelation, nodeReceived);
						
						httpResponse = new ResponseEntity<String>(HttpStatus.OK);
						
					} catch (NodeException e) {
						if(e.getType() == NodeException.NO_NODENAME_FOUND) {
							logger.warn("Node don't created yet");
							httpResponse = new ResponseEntity<String>(HttpStatus.NOT_FOUND);
						} else if (e.getType() == NodeException.TWO_NODES_WITH_SAME_NODENAME){
							logger.error("Two nodes are equals: " + HttpStatus.FORBIDDEN);
							httpResponse = new ResponseEntity<String>(HttpStatus.FORBIDDEN);
						} else {
							logger.error(String.format("Exception: %s", e.getMessage()));
							httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
						}
						e.printStackTrace();
					} catch (Exception e) {
						e.printStackTrace();
						httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
					}
					
				}

				
			} catch (InvalidKeyException | NoSuchPaddingException | NoSuchAlgorithmException | InvalidAlgorithmParameterException | BadPaddingException | IllegalBlockSizeException e) {
				httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
				logger.error(String.format("Exception: %s", e.getMessage()));
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		} catch (UserNodeRelationsException e) {
			logger.error(String.format("Node unautorized: %s", e.getMessage()));
			e.printStackTrace();
			httpResponse = new ResponseEntity<String>(HttpStatus.UNAUTHORIZED);
		}  catch (NodeException e) {
			if(e.getType() == NodeException.NODE_NOT_FOUND) {
				logger.error(String.format("Exception: %s", e.getMessage()));
			}
			e.printStackTrace();
			httpResponse = new ResponseEntity<String>(HttpStatus.BAD_REQUEST);
		}
		
				
		return httpResponse;
	}
}


