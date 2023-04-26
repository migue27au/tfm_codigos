package es.ujaen.service;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Optional;

import javax.servlet.http.HttpServletRequest;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

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

@Service
public class UserService{
	
	private static final Logger logger = LoggerFactory.getLogger(UserService.class);
	
	@Autowired
	private UserRepository userRepository;
	
	@Autowired
    private LoginAttemptService loginAttemptService;
 
    @Autowired
    private HttpServletRequest request;
	
	public UserInfo getUserInfoByUsername(String username) throws UserException {
		Optional<User> user = userRepository.findByUsername(username);
		user.orElseThrow(() -> new UserException(UserException.USER_NOT_FOUND));
		
		return new UserInfo(user.get());
	}
	
	public UserInfo getUserInfoById(long id) throws UserException {
		Optional<User> user = userRepository.findById(id);
		user.orElseThrow(() -> new UserException(UserException.USER_NOT_FOUND));
		
		return new UserInfo(user.get());
	}
	
	public void blockUserByUsername(String username) throws UserException {
		Optional<User> user = userRepository.findByUsername(username);
		user.orElseThrow(() -> new UserException(UserException.USER_NOT_FOUND));
		
		user.get().setActive(false);
		
		userRepository.save(user.get());
	}
	
	public boolean isActive(String username) throws UserException {
		Optional<User> user = userRepository.findByUsername(username);
		user.orElseThrow(() -> new UserException(UserException.USER_NOT_FOUND));
		
		return user.get().isActive();
	}
}
