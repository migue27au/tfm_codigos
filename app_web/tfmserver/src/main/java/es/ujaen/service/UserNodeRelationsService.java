package es.ujaen.service;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import es.ujaen.DTO.EditNodeDTO;
import es.ujaen.DTO.NodeInfoDTO;
import es.ujaen.DTO.PullDataDTO;
import es.ujaen.DTO.UploadDataDTO;
import es.ujaen.Exceptions.NodeException;
import es.ujaen.Exceptions.UserException;
import es.ujaen.Exceptions.UserNodeRelationsException;
import es.ujaen.entity.Node;
import es.ujaen.entity.NodeInfo;
import es.ujaen.entity.UserInfo;
import es.ujaen.entity.UserNodeRelations;
import es.ujaen.entity.UserNodeRelationsInfo;
import es.ujaen.repository.NodeRepository;
import es.ujaen.repository.UserNodeRelationsRepository;
import es.ujaen.utils.SHA256Util;

@Service
public class UserNodeRelationsService {

	private static final Logger logger = LoggerFactory.getLogger(UserNodeRelationsService.class);

	
	@Autowired
	UserNodeRelationsRepository userNodeRelationsRepository;
	
	@Autowired
	NodeService nodeService;
	
	@Autowired
	UserService userService;
	
	/*
	 * Dado un authName, devuelve el userNodeRelationsInfo que contenga el authname especificado
	 */
	public UserNodeRelationsInfo getByAuthName(String authName) throws UserNodeRelationsException, NodeException, UserException {
		Optional<UserNodeRelations> userNodeOptional = userNodeRelationsRepository.findByAuthName(authName);
		userNodeOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
		
		UserInfo userInfo = userService.getUserInfoById(userNodeOptional.get().getUserId());
		
		NodeInfo masterNode = nodeService.getNodeInfoById(userNodeOptional.get().getNodeId());
		
		String[] nodeIds = userNodeOptional.get().getNodeIds().split(UserNodeRelations.NODES_SPLIT);
		
		List<NodeInfo> nodes = new ArrayList<>();
		
		for(String id: nodeIds) {
			try {
				if(id != null) {
					logger.debug("nodeId: " + id);
					NodeInfo newNodeInfo = nodeService.getNodeInfoById(Long.parseLong(id));
					
					nodes.add(newNodeInfo);
				}
			} catch(NumberFormatException ex) {
				logger.debug(String.format("Exception NumberFormatException: ", ex.toString()));
			}
			
		}
		
		UserNodeRelationsInfo userNodeRelations = new UserNodeRelationsInfo(userNodeOptional.get().getId(), userInfo, masterNode, nodes, userNodeOptional.get().getAuthName(), userNodeOptional.get().getAuthPass());

		
		return userNodeRelations;
	}
	
	/*
	 * Dado un id, devuelve el userNodeRelationsInfo que contenga el authname especificado
	 */
	public UserNodeRelationsInfo getById(long id) throws UserNodeRelationsException, NodeException, UserException {
		Optional<UserNodeRelations> userNodeOptional = userNodeRelationsRepository.findById(id);
		userNodeOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
		
		UserInfo userInfo = userService.getUserInfoById(userNodeOptional.get().getUserId());
		
		//TODO: arreglar fallo de aquí
		NodeInfo masterNode = nodeService.getNodeInfoById(userNodeOptional.get().getNodeId());
		
		String[] nodeIds = userNodeOptional.get().getNodeIds().split(UserNodeRelations.NODES_SPLIT);
		
		List<NodeInfo> nodes = new ArrayList<>();
		
		for(String nodeId: nodeIds) {
			try {
				if(nodeId != null) {
					logger.debug("nodeId: " + nodeId);
					NodeInfo newNodeInfo = nodeService.getNodeInfoById(Long.parseLong(nodeId));
					
					nodes.add(newNodeInfo);
				}
			} catch(NumberFormatException ex) {
				logger.debug(String.format("Exception NumberFormatException: ", ex.toString()));
			}
			
		}
		
		UserNodeRelationsInfo userNodeRelations = new UserNodeRelationsInfo(userNodeOptional.get().getId(), userInfo, masterNode, nodes, userNodeOptional.get().getAuthName(), userNodeOptional.get().getAuthPass());

		
		return userNodeRelations;
	}
	
	public boolean checkIfExists(String authName) {
		Optional<UserNodeRelations> userNodeOptional = userNodeRelationsRepository.findByAuthName(authName);
		
		return userNodeOptional.isPresent();
		
	}
	
	public UserNodeRelations save(UserNodeRelations userNodeRelations) {
		return userNodeRelationsRepository.save(userNodeRelations);
	}
	
	public void save(UserNodeRelationsInfo userNodeRelationsInfo) throws UserNodeRelationsException {
		Optional<UserNodeRelations> userNodeRelationsOptional = userNodeRelationsRepository.findById(userNodeRelationsInfo.getId());
		userNodeRelationsOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
		
		userNodeRelationsOptional.get().update(userNodeRelationsInfo);
		
		userNodeRelationsRepository.save(userNodeRelationsOptional.get());
	}
	
	
	public UserNodeRelationsInfo authenticate(UploadDataDTO uploadData) throws NodeException, UserException, UserNodeRelationsException {
		Optional<UserNodeRelations> userNodeOptional = userNodeRelationsRepository.findByAuthName(uploadData.getAuthName());
		userNodeOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
		
		logger.debug("userNodeRelation: " + userNodeOptional.get().toString() );
		logger.debug("uploadData: " + uploadData.toString());
		
		if(SHA256Util.sha256Hex(userNodeOptional.get().getAuthPass().toUpperCase() + uploadData.getNonce())
				.equalsIgnoreCase(uploadData.getHash())) {	//Autenticación
			return getByAuthName(uploadData.getAuthName());
		} else {
			throw new UserNodeRelationsException(UserNodeRelationsException.BAD_AUTHENTICATION);
		}
	}
	
	/*
	 * añade un nodo a la lista de nodos y devuelve true si el nodo no existía y por tanto lo añade o false si el nodo ya estaba añadido
	 */
	public boolean addNodeToNodeList(UserNodeRelationsInfo userNodeRelations, NodeInfoDTO nodeInfoDTO) throws NodeException, UserException, UserNodeRelationsException {
		NodeInfo nodeInfo = nodeService.getNodeInfoByNodeName(nodeInfoDTO.getNodeName());
		
		if(userNodeRelations.checkIfNodeIsInNodeList(nodeInfo) == false) {
			userNodeRelations.getNodes().add(nodeInfo);
			
			save(userNodeRelations);
			return true;
		} else {
			return false;
		}
	}
	
	/*
	 * elimina un nodoId de la lista de nodesId dado un deleteNodeDTO
	 */
	public void removeNodeFromNodeList(UserNodeRelationsInfo userNodeRelationsInfo, EditNodeDTO deleteNodeDTO) throws NodeException, UserException, UserNodeRelationsException {
		NodeInfo deleteNode = nodeService.getNodeInfoById(deleteNodeDTO.getNodeId());
		
		if(userNodeRelationsInfo.checkIfNodeIsInNodeList(deleteNode) == true) {
			Optional<UserNodeRelations> userNodeRelationOptional = userNodeRelationsRepository.findById(userNodeRelationsInfo.getId());
			userNodeRelationOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
			
			userNodeRelationOptional.get().setNodeIds(userNodeRelationsInfo.getNodeIdsString().replace(UserNodeRelations.NODES_SPLIT + deleteNode.getId() + UserNodeRelations.NODES_SPLIT, UserNodeRelations.NODES_SPLIT));
			
			save(userNodeRelationOptional.get());
		} else {
			throw new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND);
		}
	}
	
	public void removeNodeFromNodeList(UserNodeRelationsInfo userNodeRelationsInfo, NodeInfo deleteNode) throws NodeException, UserException, UserNodeRelationsException {		
		if(userNodeRelationsInfo.checkIfNodeIsInNodeList(deleteNode) == true) {
			Optional<UserNodeRelations> userNodeRelationOptional = userNodeRelationsRepository.findById(userNodeRelationsInfo.getId());
			userNodeRelationOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
			
			userNodeRelationOptional.get().setNodeIds(userNodeRelationsInfo.getNodeIdsString().replace(UserNodeRelations.NODES_SPLIT + deleteNode.getId() + UserNodeRelations.NODES_SPLIT, UserNodeRelations.NODES_SPLIT));
			
			save(userNodeRelationOptional.get());
		} else {
			throw new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND);
		}
	}
	
	/*
	 * elimina un nodoId de la lista de nodesId dado un nodeId
	 */
	public void removeNodeFromNodeList(UserNodeRelationsInfo userNodeRelationsInfo, long nodeId) throws NodeException, UserException, UserNodeRelationsException {
		NodeInfo deleteNode = nodeService.getNodeInfoById(nodeId);
		
		if(userNodeRelationsInfo.checkIfNodeIsInNodeList(deleteNode) == true) {
			Optional<UserNodeRelations> userNodeRelationOptional = userNodeRelationsRepository.findById(userNodeRelationsInfo.getId());
			userNodeRelationOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
			
			userNodeRelationOptional.get().setNodeIds(userNodeRelationsInfo.getNodeIdsString().replace(UserNodeRelations.NODES_SPLIT + deleteNode.getId() + UserNodeRelations.NODES_SPLIT, UserNodeRelations.NODES_SPLIT));
			
			save(userNodeRelationOptional.get());
		} else {
			throw new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND);
		}
	}
	
	/*
	 * dado un nodeId devuelve el usernoderelations que contenga ese nodo en el campo node_ids
	 */
	public UserNodeRelationsInfo getUserNodeRelationsInfoByNodeId(long nodeId) throws UserException, UserNodeRelationsException, NodeException{
		
		logger.debug(String.format("Given id: %d", nodeId));
		String pattern = UserNodeRelations.NODES_SPLIT + nodeId + UserNodeRelations.NODES_SPLIT;
		
		Optional<UserNodeRelations> userNodeRelationsOptional = userNodeRelationsRepository.findByNodeIdsContains(pattern); 
		userNodeRelationsOptional.orElseThrow(() -> new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_NOT_FOUND));
		
		return getById(userNodeRelationsOptional.get().getId());
	}
	
	/*
	 * comprueba si existe un userNodeRelations que contenga el nodeId pasado en el campo nodes
	 */
	public boolean checkIfExistsByNodeIdInNodes(long nodeId) {
		String pattern = UserNodeRelations.NODES_SPLIT + nodeId + UserNodeRelations.NODES_SPLIT;
		
		Optional<UserNodeRelations> userNodeRelationsOptional = userNodeRelationsRepository.findByNodeIdsContains(pattern); 
		
		return userNodeRelationsOptional.isPresent();

	}
	
}
