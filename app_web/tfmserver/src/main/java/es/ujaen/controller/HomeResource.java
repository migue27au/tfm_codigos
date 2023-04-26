package es.ujaen.controller;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.nio.charset.StandardCharsets;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.TimeZone;

import javax.crypto.spec.IvParameterSpec;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import javax.validation.Valid;
import javax.validation.ValidationException;
import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.io.InputStreamResource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.annotation.AuthenticationPrincipal;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.BindException;
import org.springframework.validation.FieldError;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseStatus;
import org.springframework.web.method.annotation.MethodArgumentTypeMismatchException;
import org.springframework.web.servlet.mvc.support.DefaultHandlerExceptionResolver;

import com.google.common.hash.Hashing;

import es.ujaen.DTO.AddNodeDTO;
import es.ujaen.DTO.DeleteSharedNodesDTO;
import es.ujaen.DTO.EditNodeDTO;
import es.ujaen.DTO.NewPasswordDTO;
import es.ujaen.DTO.ShareNodeDTO;
import es.ujaen.Exceptions.NodeException;
import es.ujaen.Exceptions.PoliceException;
import es.ujaen.Exceptions.UserException;
import es.ujaen.Exceptions.UserNodeRelationsException;
import es.ujaen.entity.DataInfo;
import es.ujaen.entity.MyUserDetails;
import es.ujaen.entity.Node;
import es.ujaen.entity.NodeInfo;
import es.ujaen.entity.UserInfo;
import es.ujaen.entity.UserNodeRelations;
import es.ujaen.entity.UserNodeRelationsInfo;
import es.ujaen.repository.DataRepository;
import es.ujaen.repository.NodeRepository;
import es.ujaen.service.DataService;
import es.ujaen.service.MyUserDetailsService;
import es.ujaen.service.NodeService;
import es.ujaen.service.UserNodeRelationsService;
import es.ujaen.service.UserService;
import es.ujaen.utils.AESUtil;
import es.ujaen.utils.CookieReaderUtil;
import es.ujaen.utils.PaddingUtil;
import es.ujaen.utils.SanitizeUtil;
import es.ujaen.utils.SerializeUtil;

@Controller
@Validated
public class HomeResource {
	
	public static final String SESSION_COOKIE_NAME = "SessionCookie";
	public static final String CRYPTO_ALGORITHM = "AES/CBC/PKCS5Padding";
	public static final String SPLIT_STRING = "-/sp/-";
	public static final int SESSION_TIME = 15*60; //seconds
	public static final int POLICE_DANGER = 1;
	public static final int POLICE_WARNING = 2;
	
	public static final int NODE_INFO_NODE_ADDED_SUCCESSFULLY = 100;
	public static final int NODE_INFO_NODES_SHARED_SUCCESSFULLY = 101;
	public static final int NODE_INFO_DELETE_SHARED_NODES_SUCCESSFULLY = 102;
	public static final int NODE_INFO_EDIT_NODE_SUCCESSFULLY = 103;
	public static final int NODE_INFO_DELETE_NODE_SUCCESSFULLY = 104;
	
	public static final int NODE_WARNING_NODE_ALREADY_ADDED = 200;
	public static final int NODE_WARNING_CANNOT_SHARE_USER_NOT_FOUND = 201;
	public static final int NODE_WARNING_NODE_ALREADY_SHARED_WITH_USER = 202;
	public static final int NODE_WARNING_NONE_NODES_SELECTED_TO_SHARE = 203;
	public static final int NODE_WARNING_NONE_USER_SELECTED_TO_DELETE_SHARE = 204;
	
	public static final int NODE_ERROR_AUTHNAME_NOT_FOUND = 300;
	public static final int NODE_ERROR_SOMETHING_HAPPEND = 301;
	public static final int NODE_ERROR_CANNONT_SHARE_TO_SELF = 302;
	public static final int NODE_ERROR_WRONG_VALIDATION = 303;
	public static final int NODE_ERROR_EXISTS = 304;
	public static final int NODE_LIMIT_HAVE_BEEN_REACHED = 305;
	
	public static final int PASSWORD_DO_NOT_MATCH = 400;
	public static final int PASSWORD_CHANGE_SUCCESSFULY = 401;
	public static final int PASSWORD_IS_NOT_COMPLIANCE = 402;
	public static final int PASSWORD_COULD_NOT_BE_CHANGED = 403;
	

	public static final int MAX_NODES_PER_USER = 20;
	
	public static final boolean SECURITY_ACTIONS_ACTIVADED = true;
	
	@Autowired
	private NodeService nodeService;
	
	@Autowired
	private DataService dataService;
	
	@Autowired
	private UserNodeRelationsService userNodeRelationsService;
	
	@Autowired
	private UserService userService;
	
	@Autowired
	private MyUserDetailsService myUserDetailsService;
	
	//logger
	private static final Logger logger = LoggerFactory.getLogger(HomeResource.class);
	
	@GetMapping({"/", "home"})
	public String home(@RequestParam(value = "error", defaultValue = "false") boolean error, 
			@RequestParam(value = "logout", defaultValue = "false") boolean logout, 
			@RequestParam(value = "expired", defaultValue = "false") boolean expired, 
			@RequestParam(value = "concurrent", defaultValue = "false") boolean concurrent, 
			@RequestParam(value = "blocked", defaultValue = "false") boolean blocked, Model model) {
		
		logger.info("----------GET HOME----------");
		
		model.addAttribute("error", error);
		model.addAttribute("logout", logout);
		model.addAttribute("expired", expired);
		model.addAttribute("concurrent", concurrent);
		model.addAttribute("blocked", blocked);
		return "home";
	}
	
	
	@GetMapping("/nodes")
	public String nodes(@RequestParam(value = "msg", defaultValue = "0") @Min(0) int msg, 
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) {	    
	    

		
		logger.info("----------GET NODES----------");
		
		model.addAttribute("msg", msg);
		
		List<NodeInfo> myNodes;
		try {
			myNodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
			model.addAttribute("myNodes", myNodes);
			
			List<NodeInfo> sharedNodesWithMe = nodeService.getNodeInfosBySharedWith(userDetails.getId());
						
			model.addAttribute("sharedNodes", sharedNodesWithMe);
		} catch (UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		model.addAttribute("newNode", new AddNodeDTO());
		model.addAttribute("shareNodes", new ShareNodeDTO());
		model.addAttribute("deleteShared", new DeleteSharedNodesDTO());
		
		return "nodes";
	}
	
	
	@GetMapping("/account")
	public String account(@RequestParam(value = "msg", defaultValue = "0") @Min(0) int msg,
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req,
			HttpServletResponse res, Model model) throws PoliceException {
		
        

		
		logger.info("----------GET ACCOUNT----------");
		
		model.addAttribute("msg", msg);
		
		return "account";
		
	}
	
	@PostMapping("/changepassword")
	public String changePassword(@AuthenticationPrincipal MyUserDetails userDetails, 
			Model model, @Valid @ModelAttribute("changepassword") NewPasswordDTO newPassword) throws PoliceException {
		
		logger.info("----------POST CHANGE PASSWORD----------");
		
		logger.info("newPassword: " + newPassword.toString());
		if(newPassword.getPassword().equals(newPassword.getPasswordConfirm())) {
			if(SanitizeUtil.checkPasswordPolicy(newPassword.getPassword())) {
				try {
					myUserDetailsService.changePassword(userDetails.getId(), newPassword.getPassword());
				} catch (UserException e) {
					logger.error("Password changed error");
					return "redirect:/account?msg="+PASSWORD_COULD_NOT_BE_CHANGED;
				}
				logger.info("Password changed");
				return "redirect:/account?msg="+PASSWORD_CHANGE_SUCCESSFULY;
			} else {
				logger.info("Password is not compliance with policy");
				return "redirect:/account?msg="+PASSWORD_IS_NOT_COMPLIANCE;
			}	
		} else {
			logger.info("Passwords do not match");
			return "redirect:/account?msg="+PASSWORD_DO_NOT_MATCH;
		}		
		
	}
	
	@PostMapping("/sharenode")
	public String shareNodes(@AuthenticationPrincipal MyUserDetails userDetails, 
			Model model, @Valid @ModelAttribute("shareNodes") ShareNodeDTO sharedNodes) throws PoliceException {
		
		logger.info("----------POST SHARENODE----------");
		
		logger.debug("Shared nodes: " + sharedNodes.getUsernames());
		for(int i = 0; i < sharedNodes.getNodesList().size(); i ++) {
			logger.debug(sharedNodes.getNodesList().get(i).toString());
		}
		List<Long> nodeIds = sharedNodes.getNodesList();
		if(nodeIds.isEmpty()) {
			logger.warn("NODE_WARNING_NONE_NODES_SELECTED_TO_SHARE");
			return "redirect:/nodes?msg="+NODE_WARNING_NONE_NODES_SELECTED_TO_SHARE;
		} else {
			if(nodeService.checkIfOwns(userDetails, nodeIds)) {
				try {
					UserInfo sharedUser = userService.getUserInfoByUsername(sharedNodes.getUsernames());
					logger.debug("Shared user: " + sharedUser.toString());
					if(sharedUser.getUsername().equals(userDetails.getUsername())){
						logger.warn("NODE_ERROR_CANNONT_SHARE_TO_SELF");
						return "redirect:/nodes?msg="+NODE_ERROR_CANNONT_SHARE_TO_SELF;
					} else {
						try {
							nodeService.shareNodeWith(nodeIds, sharedUser);
							logger.info("nodes shared successfully with user " + sharedUser.getUsername());
							return "redirect:/nodes?msg="+NODE_INFO_NODES_SHARED_SUCCESSFULLY;
						} catch (NodeException e) {
							e.printStackTrace();
							logger.warn("NODE_WARNING_NODE_ALREADY_SHARED_WITH_USER");
							return "redirect:/nodes?msg="+NODE_WARNING_NODE_ALREADY_SHARED_WITH_USER;
						}
						
						
					}
				} catch (UserException e) {
					
					e.printStackTrace();
					return "redirect:/nodes?msg="+NODE_WARNING_CANNOT_SHARE_USER_NOT_FOUND;
				}
			} else {
				throw new PoliceException(PoliceException.USER_TRY_TO_SHARE_OTHERS_NODES);
			}
		}
	}
	
	@PostMapping("/addnode")
	public String addNode(@AuthenticationPrincipal MyUserDetails userDetails, 
			Model model, @Valid @ModelAttribute("newNode") AddNodeDTO newNode) throws NodeException, UserException {
		
		logger.info("----------POST ADDNODE----------");
		
		logger.debug(newNode.getNodeName().toString());
		
		List<NodeInfo> myNodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
		
		if(myNodes.size() >= MAX_NODES_PER_USER) {
			logger.warn("NODE_LIMIT_HAVE_BEEN_REACHED");
			return "redirect:/nodes?msg="+NODE_LIMIT_HAVE_BEEN_REACHED;
		} else {
			if(newNode.validation() == false) {
				logger.warn("NODE_ERROR_WRONG_VALIDATION");
				return "redirect:/nodes?msg="+NODE_ERROR_WRONG_VALIDATION;
			} else {
			
				try {
					nodeService.createNewNode(newNode.getNodeName(), newNode.getNodePass(), userDetails.getId(), newNode.isMaster(), newNode.getAuthName(), newNode.getAuthPass());
					logger.info("New node created successfuly");					
					return "redirect:/nodes?msg="+NODE_INFO_NODE_ADDED_SUCCESSFULLY;
				} catch (UserNodeRelationsException e) {	//El usuario ha metido un authName que ya existe 
					//TODO: comprobar si el authName pedido que existe pertenece al usuario y si no lanzar police exception
					e.printStackTrace();
					logger.warn("NODE_ERROR_WRONG_VALIDATION");
					return "redirect:/nodes?msg="+NODE_ERROR_WRONG_VALIDATION;
					
				} catch (NodeException e) {
					e.printStackTrace();
					if(e.getType() == NodeException.NODE_EXISTS) {
						logger.warn("NODE_ERROR_EXISTS");
						return "redirect:/nodes?msg="+NODE_ERROR_EXISTS;
					} else {
						logger.warn("NODE_ERROR_WRONG_VALIDATION");
						return "redirect:/nodes?msg="+NODE_ERROR_WRONG_VALIDATION;				
					}
					
				}	
			}	
		}
	}
	
	@PostMapping("/deleteshared")
	public String deleteShared(@RequestParam(value = "nodeId") @Min(0) @NotNull long nodeId,
			@AuthenticationPrincipal MyUserDetails userDetails, 
			Model model, @Valid @ModelAttribute("deleteShared") DeleteSharedNodesDTO deleteShared) throws PoliceException {
		
		logger.info("----------POST DELETESHARED----------");
		deleteShared.setNodeId(nodeId);
						
		logger.debug(deleteShared.toString());
		
		//return "redirect:/nodes?msg="+NODE_ERROR_AUTHNAME_NOT_FOUND;
		
		
		if(deleteShared.getUsersList().isEmpty()) {
			logger.warn("NODE_WARNING_NONE_USER_SELECTED_TO_DELETE_SHARE");
			return "redirect:/nodes?msg="+NODE_WARNING_NONE_USER_SELECTED_TO_DELETE_SHARE;
		} else {
			if(nodeService.checkIfOwns(userDetails, deleteShared.getNodeId())) {
				try {
						nodeService.deleteShared(deleteShared);
						logger.info("Delete shared successfuly");
						return "redirect:/nodes?msg="+NODE_INFO_DELETE_SHARED_NODES_SUCCESSFULLY;
				} catch (NodeException e) {
					
					e.printStackTrace();
					logger.warn("NODE_ERROR_SOMETHING_HAPPEND");
					return "redirect:/nodes?msg="+NODE_ERROR_SOMETHING_HAPPEND;
				}
			} else {
				throw new PoliceException(PoliceException.USER_TRY_TO_DELETE_SHARED_OTHERS_NODES);
			}
		}
	}

	
	@GetMapping("/tables")
	public String tables(@RequestParam(value = "id", defaultValue = "0") @Min(0) @NotNull long nodeId,
			@RequestParam(value = "from", defaultValue = "00-00-0000") @Pattern(regexp = "^[0-9]{2}-[0-9]{2}-[0-9]{4}$") String fromDate,
			@RequestParam(value = "to", defaultValue = "00-00-0000") @Pattern(regexp = "^[0-9]{2}-[0-9]{2}-[0-9]{4}$") String toDate,
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) throws PoliceException {
		
        

		
		logger.info("----------GET TABLES----------");
		try {
			List<NodeInfo> nodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
			nodes.addAll(nodeService.getNodeInfosBySharedWith(userDetails.getId()));
			model.addAttribute("nodes", nodes);
		
			
			
			if(nodes.isEmpty() == false && nodeId != 0) {
				if(nodeService.checkIfOwns(userDetails, nodeId)==true || nodeService.checkIfShareWith(userDetails, nodeId) == true) {	//TODO: ¿lanzar excepción?
					NodeInfo selectedNode = nodeService.getNodeInfoById(nodeId);
					logger.debug("Selected node: " + selectedNode.toString());
					
					//TODO: arreglar fallo cuando se piden datos a las 12: police error
					
					Long unixtime_from = (long) 0;
					Long unixtime_to = (long) 0;
					String pattern = "dd-MM-yyyy";
					SimpleDateFormat simpleDateFormat = new SimpleDateFormat(pattern);
					//por defecto
					if(fromDate.matches("00-00-0000")) {
						Calendar calendar = Calendar.getInstance();
						calendar.set(Calendar.HOUR_OF_DAY, 0);
				        calendar.set(Calendar.MINUTE, 0);
				        calendar.set(Calendar.SECOND, 0);
				        calendar.set(Calendar.MILLISECOND, 0);
				        
				        unixtime_from = calendar.getTime().getTime();
					} else {
						unixtime_from = simpleDateFormat.parse(fromDate).getTime();
					}
					//por defecto
					if(toDate.matches("00-00-0000")) {
						unixtime_to = unixtime_from+24*3600*1000;
					} else {
						unixtime_to = simpleDateFormat.parse(toDate).getTime()+24*3600*1000;
					}
					
					//si la diferencia es de más de 7 días
					if(unixtime_to-unixtime_from > 7*24*3600*1000) {
						throw new PoliceException(PoliceException.USER_TRY_TO_GET_MORE_DATA);
					}
					//si alguna de las fechas es posterior al dia de hoy + 12 hora por si acaso (gmt +12)
					if(unixtime_to > Calendar.getInstance().getTime().getTime()+(24+12)*3600*1000 || unixtime_from > Calendar.getInstance().getTime().getTime()) {
						throw new PoliceException(PoliceException.USER_TRY_TO_GET_MORE_DATA);
					}
										
					logger.debug("Time from: " + unixtime_from.toString() + " to " + unixtime_to.toString());
										
					List<DataInfo> datas = dataService.getByNodeInfoOrderByTime(selectedNode, 511, unixtime_from, unixtime_to);
					//List<DataInfo> datas = dataService.getByNodeInfoOrderByTime(selectedNode);
					
					model.addAttribute("selectedNode", selectedNode);
					model.addAttribute("dataInfos", datas);
				} else {
					throw new PoliceException(PoliceException.USER_TRY_TO_GET_OTHER_NODE);
				}
			}
				
		} catch (NodeException | UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ParseException e) {
			e.printStackTrace();
			throw new PoliceException(PoliceException.USER_TRY_TO_GET_OTHER_NODE);
		}
				
		return "tables";
	}
	
	@GetMapping("/editnodes")
	public String editnodes(@RequestParam(value = "id", defaultValue = "0") @Min(0) @NotNull long nodeId,
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) throws PoliceException {
	
        

		
		logger.info("----------GET EDITNODE----------");
		
		NodeInfo selectedNode = new NodeInfo();	
		
		try {
			List<NodeInfo> nodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
			model.addAttribute("nodes", nodes);
			
			if(nodes.isEmpty() == false && nodeId != 0) {
				if(nodeService.checkIfOwns(userDetails, nodeId)==true || nodeService.checkIfShareWith(userDetails, nodeId) == true) {
					selectedNode = nodeService.getNodeInfoById(nodeId);
					logger.debug("Selected node: " + selectedNode.toString());

					
					model.addAttribute("selectedNode", selectedNode);
				} else {
					throw new PoliceException(PoliceException.USER_TRY_TO_EDIT_OTHER_NODE);
				}
			}
				
		} catch (NodeException | UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		model.addAttribute("editNodeDTO", new EditNodeDTO(selectedNode));
		return "editNode";
	}
	
	@PostMapping("/editnode")
	public String editNode(@RequestParam(value = "nodeId") @Min(0) @NotNull long nodeId,
			@AuthenticationPrincipal MyUserDetails userDetails, 
			Model model, @Valid @ModelAttribute("editNodeDTO") EditNodeDTO editNode) throws PoliceException {
		
		logger.info("----------POST EDITNODE----------");
		
		editNode.setNodeId(nodeId);
						
		logger.debug("Edit node: " + editNode.toString());
		
		if(nodeService.checkIfOwns(userDetails, editNode.getNodeId())) {
			try {
				nodeService.editNode(editNode);
				logger.info("Node edited successfuly");
				return "redirect:/nodes?msg="+NODE_INFO_EDIT_NODE_SUCCESSFULLY;
			} catch (NodeException e) {
				
				e.printStackTrace();
				logger.warn("NODE_ERROR_SOMETHING_HAPPEND");
				return "redirect:/nodes?msg="+NODE_ERROR_SOMETHING_HAPPEND;
			}
		} else {
			throw new PoliceException(PoliceException.USER_TRY_TO_EDIT_OTHER_NODE);
		}
	}
	
	@GetMapping("/deletenodes")
	public String deletenodes(@RequestParam(value = "id", defaultValue = "0") @Min(0) @NotNull long nodeId,
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) throws PoliceException {
	
        

		
		logger.info("----------GET DELETENODES----------");
		
		NodeInfo selectedNode = new NodeInfo();	
			
		try {
			List<NodeInfo> myNodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
			model.addAttribute("mynodes", myNodes);
			
			List<NodeInfo> sharedNodes = nodeService.getNodeInfosBySharedWith(userDetails.getId());
			model.addAttribute("sharednodes", sharedNodes);
			
			if(myNodes.isEmpty() == false && nodeId != 0) {
				//compruebo si pertenece al usuario o si
				if(nodeService.checkIfOwns(userDetails, nodeId)==true || nodeService.checkIfShareWith(userDetails, nodeId) == true) {	//TODO: ¿lanzar excepción?
					selectedNode = nodeService.getNodeInfoById(nodeId);
					logger.debug("Selected node: " + selectedNode.toString());

					
					model.addAttribute("selectedNode", selectedNode);
				} else {
					throw new PoliceException(PoliceException.USER_TRY_TO_DELETE_OTHER_NODE);
				}
			}
				
		} catch (NodeException | UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		model.addAttribute("deleteNodeDTO", new EditNodeDTO(selectedNode));
		return "deleteNodes";
	}
	
	@PostMapping("/deletenode")
	public String deletenode(@RequestParam(value = "nodeId") @Min(0) @NotNull long nodeId,
			@AuthenticationPrincipal MyUserDetails userDetails, 
			Model model) throws PoliceException{
		
		NodeInfo deleteNode;
		try {
			deleteNode = nodeService.getNodeInfoById(nodeId);
			logger.info("----------POST DELETENODE----------");
			logger.debug("Deleted node: " + deleteNode.toString());
			
			//compruebo si pertenece al que quiere borrar
			if(nodeService.checkIfOwns(userDetails, deleteNode.getId())) {
				try {
					try {
						if(userNodeRelationsService.checkIfExistsByNodeIdInNodes(deleteNode.getId())) {
							UserNodeRelationsInfo userNodeRelations = userNodeRelationsService.getUserNodeRelationsInfoByNodeId(deleteNode.getId());
							userNodeRelationsService.removeNodeFromNodeList(userNodeRelations, deleteNode);
							logger.info("Nodo " + deleteNode.getId() + " eliminado de userNodeRelations");
						} else {
							logger.info("Nodo " + deleteNode.getId() + " no encontrado en userNodeRelations");
						}
					} catch (UserNodeRelationsException | UserException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					} //obtengo el userNodeRelations que contiene el nodo
					
					nodeService.deleteNode(deleteNode);	//borro el node
					logger.info("node deleted successfuly");
					return "redirect:/nodes?msg="+NODE_INFO_DELETE_NODE_SUCCESSFULLY;
				} catch (NodeException e) {
					
					e.printStackTrace();
					logger.warn("NODE_ERROR_SOMETHING_HAPPEND");
					return "redirect:/nodes?msg="+NODE_ERROR_SOMETHING_HAPPEND;
				}
			//si no compruebo si el nodo está compartido con el usuario
			} else if(nodeService.checkIfShareWith(userDetails, nodeId) == true){
				//si el nodo está compartido con el usuario dejo de compartirlo
				try {
					nodeService.deleteShared(nodeId, userDetails.getId());
					logger.warn("NODE_INFO_DELETE_NODE_SUCCESSFULLY");
					return "redirect:/nodes?msg="+NODE_INFO_DELETE_NODE_SUCCESSFULLY;
				} catch (NodeException e) {
					e.printStackTrace();
					logger.warn("NODE_ERROR_SOMETHING_HAPPEND");
					return "redirect:/nodes?msg="+NODE_ERROR_SOMETHING_HAPPEND;
				}
				
			} else {
				throw new PoliceException(PoliceException.USER_TRY_TO_DELETE_OTHER_NODE);
			}
		} catch (NodeException | UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			logger.warn("NODE_ERROR_SOMETHING_HAPPEND");
			return "redirect:/nodes?msg="+NODE_ERROR_SOMETHING_HAPPEND;
		}
	}
	
	@GetMapping("/charts")
	public String charts(@RequestParam(value = "id", defaultValue = "0") @Min(0) @NotNull long nodeId,
			@RequestParam(value = "values", defaultValue = "511") @Min(0) @Max(511) @NotNull int searchValues,
			@RequestParam(value = "from", defaultValue = "00-00-0000") @Pattern(regexp = "^[0-9]{2}-[0-9]{2}-[0-9]{4}$") String fromDate,
			@RequestParam(value = "to", defaultValue = "00-00-0000") @Pattern(regexp = "^[0-9]{2}-[0-9]{2}-[0-9]{4}$") String toDate,
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) throws PoliceException {
		
        

		
		logger.info("----------GET CHARTS----------");
		
		try {
			List<NodeInfo> nodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
			nodes.addAll(nodeService.getNodeInfosBySharedWith(userDetails.getId()));
			model.addAttribute("nodes", nodes);
		
			if(nodes.isEmpty() == false && nodeId != 0) {
				if(nodeService.checkIfOwns(userDetails, nodeId)==true || nodeService.checkIfShareWith(userDetails, nodeId) == true) {
					NodeInfo selectedNode = nodeService.getNodeInfoById(nodeId);
					logger.debug("Selected node: " + selectedNode.toString());

					
					Long unixtime_from = (long) 0;
					Long unixtime_to = (long) 0;
					String pattern = "dd-MM-yyyy";
					SimpleDateFormat simpleDateFormat = new SimpleDateFormat(pattern);
					
					//por defecto
					if(fromDate.matches("00-00-0000")) {
						Calendar calendar = Calendar.getInstance();
						calendar.set(Calendar.HOUR_OF_DAY, 0);
				        calendar.set(Calendar.MINUTE, 0);
				        calendar.set(Calendar.SECOND, 0);
				        calendar.set(Calendar.MILLISECOND, 0);
				        
				        unixtime_from = calendar.getTime().getTime();
					} else {
						unixtime_from = simpleDateFormat.parse(fromDate).getTime();
					}
					//por defecto
					if(toDate.matches("00-00-0000")) {
						unixtime_to = unixtime_from+24*3600*1000;
					} else {
						unixtime_to = simpleDateFormat.parse(toDate).getTime()+24*3600*1000;
					}
					
					//si la diferencia es de más de 7 días
					if(unixtime_to-unixtime_from > 7*24*3600*1000) {
						throw new PoliceException(PoliceException.USER_TRY_TO_GET_MORE_DATA);
					}
					//si alguna de las fechas es posterior al dia de hoy + 12 hora por si acaso (gmt +12)
					if(unixtime_to > Calendar.getInstance().getTime().getTime()+(24+12)*3600*1000 || unixtime_from > Calendar.getInstance().getTime().getTime()) {
						throw new PoliceException(PoliceException.USER_TRY_TO_GET_MORE_DATA);
					}
										
					logger.info("Time from: " + unixtime_from.toString() + " to " + unixtime_to.toString());

										
					List<DataInfo> datas = dataService.getByNodeInfoOrderByTime(selectedNode, searchValues, unixtime_from, unixtime_to);
					//List<DataInfo> datas = dataService.getByNodeInfoOrderByTime(selectedNode);
					
					model.addAttribute("selectedNode", selectedNode);
					model.addAttribute("dataInfos", datas);
				} else {
					throw new PoliceException(PoliceException.USER_TRY_TO_GET_OTHER_NODE);
				}
			}
				
		} catch (NodeException | UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ParseException e) {
			e.printStackTrace();
			throw new PoliceException(PoliceException.USER_TRY_TO_GET_OTHER_NODE);
		}
				
		return "charts";
	}
	
	@GetMapping("/exports")
	public String exports(@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) throws PoliceException {
		
        
		
		logger.info("----------GET EXPORTS----------");
		try {
			List<NodeInfo> nodes = nodeService.getNodeInfosByOwnerId(userDetails.getId());
			nodes.addAll(nodeService.getNodeInfosBySharedWith(userDetails.getId()));
			model.addAttribute("nodes", nodes);
		} catch (UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return "exports";
	}
	
	@PostMapping("/export")
	public ResponseEntity<InputStreamResource> export(@RequestParam(value = "nodeId", defaultValue = "0") @Min(0) @NotNull long nodeId,
			@RequestParam(value = "from", defaultValue = "00-00-0000") @Pattern(regexp = "^[0-9]{2}-[0-9]{2}-[0-9]{4}$") @NotNull String fromDate,
			@RequestParam(value = "to", defaultValue = "00-00-0000") @Pattern(regexp = "^[0-9]{2}-[0-9]{2}-[0-9]{4}$") @NotNull String toDate,
			@AuthenticationPrincipal MyUserDetails userDetails, HttpServletRequest req, HttpServletResponse res, Model model) throws PoliceException {
		
		logger.info("----------POST EXPORT----------");
		
		NodeInfo nodeInfo;
		try {
			nodeInfo = nodeService.getNodeInfoById(nodeId);
			
			if(nodeService.checkIfOwns(userDetails, nodeId)==true || nodeService.checkIfShareWith(userDetails, nodeId) == true) {
				NodeInfo selectedNode = nodeService.getNodeInfoById(nodeId);
				logger.debug("Selected node: " + selectedNode.toString());

				
				Long unixtime_from = (long) 0;
				Long unixtime_to = (long) 0;
				String pattern = "dd-MM-yyyy";
				SimpleDateFormat simpleDateFormat = new SimpleDateFormat(pattern);
				//por defecto
				if(fromDate.matches("00-00-0000")) {
					Calendar calendar = Calendar.getInstance();
					calendar.set(Calendar.HOUR_OF_DAY, 0);
			        calendar.set(Calendar.MINUTE, 0);
			        calendar.set(Calendar.SECOND, 0);
			        calendar.set(Calendar.MILLISECOND, 0);
			        
			        unixtime_from = calendar.getTime().getTime();
				} else {
					unixtime_from = simpleDateFormat.parse(fromDate).getTime();
				}
				//por defecto
				if(toDate.matches("00-00-0000")) {
					unixtime_to = unixtime_from+24*3600*1000;
				} else {
					unixtime_to = simpleDateFormat.parse(toDate).getTime()+24*3600*1000;
				}
				
				//si alguna de las fechas es posterior al dia de hoy + 12 hora por si acaso (gmt +12)
				if(unixtime_to > Calendar.getInstance().getTime().getTime()+(24+12)*3600*1000 || unixtime_from > Calendar.getInstance().getTime().getTime()) {
					throw new PoliceException(PoliceException.USER_TRY_TO_GET_MORE_DATA);
				}
									
				logger.debug("Time from: " + unixtime_from.toString() + " to " + unixtime_to.toString());

									
				ByteArrayInputStream ais = dataService.exportAllData(nodeInfo, unixtime_from, unixtime_to);
				
				//nombre del archivo --> nodename(alias)-20210813164800
				Date date = Calendar.getInstance().getTime();  
				DateFormat dateFormat = new SimpleDateFormat("yyyyMMddhhmmss");  
				String strDate = dateFormat.format(date);  
				
				String filename = nodeInfo.getNodeName() + "(" + nodeInfo.getAlias() + ")-"+strDate +".xlsx";
				
				HttpHeaders headers = new HttpHeaders();
				headers.add("Content-Disposition", "attachment; filename=" + filename);
				headers.set("Contenty-Type", "application/x-excel");
				
				
				logger.info("Export completed successfuly");
				return ResponseEntity.ok().headers(headers).body(new InputStreamResource(ais));
			
				
			//si no tiene acceso al nodo devuelve excepción
			} else {
				throw new PoliceException(PoliceException.USER_TRY_TO_EXPORT_OTHER_NODE);
			}
		
		} catch (NodeException | UserException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (ParseException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			throw new PoliceException(PoliceException.USER_TRY_TO_GET_OTHER_NODE);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		
		return new ResponseEntity<InputStreamResource>(HttpStatus.BAD_REQUEST);
	}
	
	
	
//	@ResponseStatus(HttpStatus.BAD_REQUEST)
//	@ExceptionHandler(MethodArgumentNotValidException.class)
//	public Map<String, String> handleValidationExceptions(MethodArgumentNotValidException ex) {
//	    Map<String, String> errors = new HashMap<>();
//	    ex.getBindingResult().getAllErrors().forEach((error) -> {
//	        String fieldName = ((FieldError) error).getField();
//	        String errorMessage = error.getDefaultMessage();
//	        System.out.println("Excepcion: " + errorMessage);
//	        errors.put(fieldName, errorMessage);
//	    });
//	    
//	    
//	    return errors;
//	}
	
	
	//###########    SANITIZATION HANDLERS    ###########
	
	@ResponseStatus(HttpStatus.BAD_REQUEST)
	@ExceptionHandler(MethodArgumentTypeMismatchException.class)
	public String argumentTypeMismatchException(MethodArgumentTypeMismatchException ex, HttpServletRequest req,  Model model) {
		//TODO:... log, cerrar sesión, banear, ...
		logger.error("Malicious activity from user:  " + req.getRemoteUser());
		if(SECURITY_ACTIONS_ACTIVADED) {
			HttpSession session = req.getSession(false);
			if (session != null) {
				session.invalidate();
				logger.error("User session have been invalidated");
			}
			try {
				userService.blockUserByUsername(req.getRemoteUser());
				logger.error("User account have been blocked");
			} catch (UserException e) {
				logger.warn("Cannot block user");
				e.printStackTrace();
			}
		}
		logger.error("EXCEPCION : " + ex.getMessage());
		
		model.addAttribute("error", POLICE_DANGER);
		return "error/police";
	}
	
	@ResponseStatus(HttpStatus.BAD_REQUEST)
	@ExceptionHandler(BindException.class)
	public String validationException(BindException ex, HttpServletRequest req, Model model) {
		//TODO:... log, cerrar sesión, banear, ...
		logger.error("Malicious activity from user:  " + req.getRemoteUser());
		if(SECURITY_ACTIONS_ACTIVADED) {
			HttpSession session = req.getSession(false);
			if (session != null) {
				session.invalidate();
				logger.error("User session have been invalidated");
			}
			try {
				userService.blockUserByUsername(req.getRemoteUser());
				logger.error("User account have been blocked");
			} catch (UserException e) {
				logger.warn("Cannot block user");
				e.printStackTrace();
			}
		}
		
		logger.error("EXCEPCION : " + ex.getMessage());
		model.addAttribute("error", POLICE_DANGER);
		return "error/police";
	}
	
	
	@ResponseStatus(HttpStatus.BAD_REQUEST)
	@ExceptionHandler(ValidationException.class)
	public String validationException(ValidationException ex, HttpServletRequest req,  Model model) {
		//TODO:... log, cerrar sesión, banear, ...
		logger.error("Malicious activity from user:  " + req.getRemoteUser());
		if(SECURITY_ACTIONS_ACTIVADED) {
			HttpSession session = req.getSession(false);
			if (session != null) {
				session.invalidate();
				logger.error("User session have been invalidated");
			}
		}
		
		logger.error("EXCEPCION : " + ex.getMessage());
		model.addAttribute("error", POLICE_WARNING);
		return "error/police";
	}
	
	
	@ResponseStatus(HttpStatus.BAD_REQUEST)
	@ExceptionHandler(PoliceException.class)
	public String handlePoliceException(PoliceException pex, HttpServletRequest req, Model model) {
		//TODO:... log, cerrar sesión, banear, ...
		logger.error("Malicious activity from user:  " + req.getRemoteUser());
		if(SECURITY_ACTIONS_ACTIVADED) {
			HttpSession session = req.getSession(false);
			if (session != null) {
				session.invalidate();
				logger.error("User session have been invalidated");
			}
		}
		logger.error("EXCEPCION : " + pex.getMessage());
		
		switch(pex.getType()) {
			case PoliceException.USER_CAUSE_DATE_PARSE_ERROR:
				model.addAttribute("error", POLICE_DANGER);
				if(SECURITY_ACTIONS_ACTIVADED) {
					try {
						userService.blockUserByUsername(req.getRemoteUser());
						logger.error("User account have been blocked");
					} catch (UserException e) {
						logger.warn("Cannot block user");
						e.printStackTrace();
					}
				}
				break;
			
			case PoliceException.USER_TRY_TO_DELETE_SHARED_OTHERS_NODES:
			case PoliceException.USER_TRY_TO_SHARE_OTHERS_NODES:
			case PoliceException.USER_TRY_TO_EDIT_OTHER_NODE:
			case PoliceException.USER_TRY_TO_GET_MORE_DATA:
			case PoliceException.USER_TRY_TO_GET_OTHER_NODE:
				model.addAttribute("error", POLICE_WARNING);
				break;
			default:
				model.addAttribute("error", POLICE_WARNING);
				break;
				
		}
		
		return "error/police";
	}
}
