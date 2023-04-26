package es.ujaen.service;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import es.ujaen.DTO.DeleteSharedNodesDTO;
import es.ujaen.DTO.EditNodeDTO;
import es.ujaen.DTO.NodeInfoDTO;
import es.ujaen.DTO.PullDataDTO;
import es.ujaen.Exceptions.NodeException;
import es.ujaen.Exceptions.UserException;
import es.ujaen.Exceptions.UserNodeRelationsException;
import es.ujaen.entity.MyUserDetails;
import es.ujaen.entity.Node;
import es.ujaen.entity.NodeInfo;
import es.ujaen.entity.User;
import es.ujaen.entity.UserInfo;
import es.ujaen.entity.UserNodeRelations;
import es.ujaen.repository.NodeRepository;
import es.ujaen.repository.UserRepository;
import es.ujaen.utils.SHA256Util;

@Service
public class NodeService{
	
	private static final Logger logger = LoggerFactory.getLogger(NodeService.class);

	
	@Autowired
	NodeRepository nodeRepository;
	@Autowired
	UserService userService;
	@Autowired
	UserNodeRelationsService userNodeRelationsService;
	
	public Node getByNodeName(String nodeName) throws NodeException {
		Optional<Node> node = nodeRepository.findByNodeName(nodeName);
		
		node.orElseThrow(() -> new NodeException(NodeException.NO_NODENAME_FOUND));
		
		return node.get();
	}
	
	
	public long getIdByNodeName(String nodeName) throws NodeException {
		Optional<Node> node = nodeRepository.findByNodeName(nodeName);
		
		node.orElseThrow(() -> new NodeException(NodeException.NO_NODENAME_FOUND));
		
		return node.get().getId();
	}
	
	public boolean checkIfExists(String authName){
		Optional<Node> node = nodeRepository.findByNodeName(authName);
		
		return node.isPresent();
	}
	
	public List<Node> getByOwnerId(long ownerId){
		return nodeRepository.findByOwnerId(ownerId);
	}
	
	public NodeInfo getNodeInfoById(long id) throws NodeException, UserException {	
		Optional<Node> node = nodeRepository.findById(id);
		node.orElseThrow(() -> new NodeException(NodeException.NODE_NOT_FOUND));
		
		UserInfo owner = userService.getUserInfoById(node.get().getOwnerId());
		
		return new NodeInfo(node.get(), owner);
		
	}
	
	public NodeInfo getNodeInfoByNodeName(String nodeName) throws NodeException, UserException {
		Optional<Node> node = nodeRepository.findByNodeName(nodeName);
		node.orElseThrow(() -> new NodeException(NodeException.NO_NODENAME_FOUND));
		
		UserInfo owner = userService.getUserInfoById(node.get().getOwnerId());
		
		return new NodeInfo(node.get(), owner);
	}
	
	public Node getById(long id) throws NodeException {
		Optional<Node> node = nodeRepository.findById(id);
		node.orElseThrow(() -> new NodeException(NodeException.NODE_NOT_FOUND));
		return node.get(); 
	}
	
	public List<Node> getAllById(List<Long> ids){
		return nodeRepository.findAllById(ids);
	}
	
	public List<NodeInfo> getNodeInfosByOwnerId(long ownerId) throws UserException{
		List<NodeInfo> nodeInfos = new ArrayList<>();
		List<Node> nodes = getByOwnerId(ownerId);
		
		UserInfo owner = userService.getUserInfoById(ownerId);
		
		Iterator<Node> nodesIterator = nodes.iterator();
		
		while(nodesIterator.hasNext()) {
			Node n = nodesIterator.next();
			
			String[] usersId = n.getSharedWith().split(Node.SHARED_WITH_SPLIT);
			
			for(String s: usersId) {
				logger.debug(String.format("Node shared with: %s", s));
			}
			
			List<UserInfo> sharedUsers = new ArrayList<>();
			
			for(String id: usersId) {
				try {
					UserInfo newUserInfo = userService.getUserInfoById(Long.parseLong(id));
					
					sharedUsers.add(newUserInfo);
					
				} catch(NumberFormatException ex) {
					logger.debug(String.format("Exception NumberFormatException: ", ex.toString()));
					
				}
				
			}
			
			
			
			NodeInfo newNodeInfo = new NodeInfo(n, owner, sharedUsers);
			nodeInfos.add(newNodeInfo);
		}
		
		return nodeInfos;
	}
	
	public List<NodeInfo> getNodeInfosBySharedWith(long userId) throws UserException{
		List<NodeInfo> nodeInfos = new ArrayList<>();
		logger.debug(String.format("Given id: %d", userId));
		String pattern = Node.SHARED_WITH_SPLIT + userId + Node.SHARED_WITH_SPLIT;
		List<Node> nodes = nodeRepository.findBySharedWithContains(pattern);
		
		Iterator<Node> nodesIterator = nodes.iterator();
		
		while(nodesIterator.hasNext()) {
			Node n = nodesIterator.next();
			
			UserInfo owner = userService.getUserInfoById(n.getOwnerId());
			
			NodeInfo newNodeInfo = new NodeInfo(n, owner);
			nodeInfos.add(newNodeInfo);
		}
		
		return nodeInfos;
	}
	
	
	/*Dado un usuario y un nodo comprueba si el nodo pertence al usuario */
	public boolean checkIfOwns(MyUserDetails user, NodeInfo node) {		
		if(node.getOwner().getId() == user.getId()) {
			return true;
		} else {
			return false;
		}
	}
	
	public boolean checkIfOwns(long userId, long nodeId) {
		try {
			Node node = getById(nodeId);
			
			if(node.getOwnerId() == userId) {
				return true;
			} else {
				return false;
			}
		} catch (NodeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean checkIfOwns(MyUserDetails user, long nodeId) {
		try {
			Node node = getById(nodeId);
			
			if(node.getOwnerId() == user.getId()) {
				return true;
			} else {
				return false;
			}
		} catch (NodeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean checkIfOwns(MyUserDetails user, List<Long> nodeIds) {
		List<Node> nodes = nodeRepository.findAllById(nodeIds);
		if(!nodes.isEmpty()) {
			Iterator<Node> nodesIterator = nodes.iterator();
			while(nodesIterator.hasNext()) {
				if(nodesIterator.next().getOwnerId() != user.getId()) {
					return false;
				}
			}
			return true;
		} else {
			return false;
		}
	}
	
	/*
	 * Esta función comprueba si el nodo indicado está compartido con el usuario dado
	 */
	public boolean checkIfShareWith(long userId, long nodeId) {
		try {
			Node node = getById(nodeId);
			
			if(node.getSharedWith().contains("," + userId + ",")) {
				return true;
			} else {
				return false;
			}
			
		} catch (NodeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	}
	
	
	/*
	 * Esta función comprueba si el nodo indicado está compartido con el usuario dado
	 */
	public boolean checkIfShareWith(MyUserDetails user, long nodeId) {
		try {
			Node node = getById(nodeId);
			
			if(node.getSharedWith().contains("," + user.getId() + ",")) {
				return true;
			} else {
				return false;
			}
			
		} catch (NodeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	}
	
	public boolean checkIfShareWith(MyUserDetails user, NodeInfo nodeInfo) {
		try {
			Node node = getById(nodeInfo.getId());
			
			if(node.getSharedWith().contains("," + user.getId() + ",")) {
				return true;
			} else {
				return false;
			}
			
		} catch (NodeException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	}
	
	public List<Node> shareNodeWith(List<Long> nodeIds, UserInfo user) throws NodeException{
		List<Node> nodes = getAllById(nodeIds);
		
		nodes.forEach(node -> {
			try {
				node.setNewSharedWith(user.getId());
			} catch (NodeException e) {
				e.printStackTrace();
			}
		});
			
		nodeRepository.saveAll(nodes);
		return nodes;
	}
	
	public Node shareNodeWith(long nodeId, UserInfo user) throws NodeException{
		Node node = getById(nodeId);
		
		node.setNewSharedWith(user.getId());
		nodeRepository.save(node);
		return node;
	}
	
	public Node shareNodeWith(Node node, UserInfo user) throws NodeException{
		node.setNewSharedWith(user.getId());
		nodeRepository.save(node);
		return node;
	}
	
	
	/*
	 * Dada una lista de usuarios deja de compartir
	 */
	public void deleteShared(DeleteSharedNodesDTO deleteShared) throws NodeException {
		Node node = getById(deleteShared.getNodeId());
		
		deleteShared.getUsersList().forEach(userId -> {
			node.setSharedWith(node.getSharedWith().replace(Node.SHARED_WITH_SPLIT + userId + Node.SHARED_WITH_SPLIT, Node.SHARED_WITH_SPLIT));
		});
		
		nodeRepository.save(node);
	}
	
	/*
	 * Dado un usuario y un nodo deja de compartirlo
	 */
	public void deleteShared(long nodeId, long userId) throws NodeException {
		Node node = getById(nodeId);
		node.setSharedWith(node.getSharedWith().replace("," + userId + ",", ","));
		
		nodeRepository.save(node);
	}
	
	public NodeInfo authenticate(PullDataDTO pullData) throws NodeException, UserException {
		Node node = getByNodeName(pullData.getName());
		
		if(SHA256Util.sha256Hex(
				node.getNodePass().toUpperCase() + pullData.getRand())
				.equalsIgnoreCase(pullData.getHash())) {
			
			UserInfo owner = userService.getUserInfoById(node.getOwnerId());
			
			return new NodeInfo(node, owner);
		} else {
			throw new NodeException(NodeException.BAD_AUTHENTICATION);
		}
	}
	
	
	
	public void editNode(EditNodeDTO editNode) throws NodeException {
		Node node = getById(editNode.getNodeId());
		node.updateByEditNodeDTO(editNode);
		
		nodeRepository.save(node);
	}
	
	
	//Función para añadir nuevos nodos
	public void createNewNode(String nodeName, String nodePass, long ownerId, boolean isMaster, String authName, String authPass) throws UserNodeRelationsException, NodeException{
		if(checkIfExists(nodeName) == false) {
			Node node = new Node(nodeName, SHA256Util.sha256Hex(nodePass), ownerId);	//se almacena el hash de la contraseña
			
			node.setSharedWith(",");
			node.setAlias("New Node");
			if(isMaster == true) {
				node.setFunction(Node.FUNCTION_MASTER);				
			} else {
				node.setFunction(Node.FUNCTION_SLAVE);
			}
			
			nodeRepository.save(node);
			
			//Si el nodo es un master deberá crearse un objeto usernoderelations para relacionar los slaves de ese master 
			if(isMaster == true) {
				if(userNodeRelationsService.checkIfExists(authName) == false) {
					long nodeId = getIdByNodeName(node.getNodeName());
					//Creo el user node relations que relaciona el owner, el id del nodo, el authname y el hash del authpass
					UserNodeRelations userNodeRelation = new UserNodeRelations(ownerId, nodeId, ",", authName, SHA256Util.sha256Hex(authPass.toUpperCase()));	//se almacena el authhash hasheado
					//UserNodeRelations userNodeRelation = new UserNodeRelations(ownerId, nodeId, ",", authName, authPass);	//se almacena el authhash hasheado
					
					userNodeRelationsService.save(userNodeRelation);
				} else {
					nodeRepository.deleteById(getIdByNodeName(node.getNodeName()));
					throw new UserNodeRelationsException(UserNodeRelationsException.USER_NODE_RELATIONS_EXISTS);
				}
			}
		} else {
			throw new NodeException(NodeException.NODE_EXISTS);
		}
	}
	
	/*Dato un nodo lo elimina de la base de datos*/
	public void deleteNode(EditNodeDTO deleteNode) throws NodeException {
		Node node = getById(deleteNode.getNodeId());
				
		nodeRepository.delete(node);
	}
	
	public void deleteNode(NodeInfo deleteNode) throws NodeException {
		Node node = getById(deleteNode.getId());
				
		nodeRepository.delete(node);
	}
	
	public void deleteNodeById(EditNodeDTO deleteNode){
		nodeRepository.deleteById(deleteNode.getNodeId());
	}
	
	public NodeInfo updateParametersByNodeInfoDTO(NodeInfoDTO nodeInfoDTO) throws NodeException, UserException {
		Node node = getByNodeName(nodeInfoDTO.getNodeName());
		
		//compruebo si no está inicializado
		if(node.getAddressLsb() != 0 && node.getAddressMsb() != 0) {
			//compruebo si se está intentando añadir otro nodo
			if(nodeInfoDTO.getLsb() != node.getAddressLsb() || nodeInfoDTO.getMsb() != node.getAddressMsb()) {
				throw new NodeException(NodeException.TWO_NODES_WITH_SAME_MAC);
			}
		}
		
		node.updateParameters(nodeInfoDTO);	//actualizo el nodo
		nodeRepository.save(node);			//lo guardo				
		
		UserInfo owner = userService.getUserInfoById(node.getOwnerId());
		
		return new NodeInfo(node, owner);	//devuelvo el nodeInfo
	}
}
