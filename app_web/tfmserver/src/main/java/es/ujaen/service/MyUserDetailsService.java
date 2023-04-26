package es.ujaen.service;

import java.util.Optional;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

import es.ujaen.Exceptions.UserException;
import es.ujaen.entity.MyUserDetails;
import es.ujaen.entity.User;
import es.ujaen.repository.UserRepository;

@Service
public class MyUserDetailsService implements UserDetailsService{
	
	private static final Logger logger = LoggerFactory.getLogger(MyUserDetailsService.class);

	
	@Autowired
	UserRepository userRepository;
	
	@Autowired
	PasswordEncoder passwordEncoder;
	
	@Autowired
    private LoginAttemptService loginAttemptService;
	
	@Override
	public UserDetails loadUserByUsername(String username){
		if (loginAttemptService.isBlocked()) {
            throw new RuntimeException("IP BLOCKED");
        }
		
		Optional<User> user = userRepository.findByUsername(username);
		
		user.orElseThrow(() -> new UsernameNotFoundException("Not found: " + username));
		
		return user.map(MyUserDetails::new).get();
	}
	
	public void changePassword(long user_id, String password) throws UserException {
		Optional<User> user = userRepository.findById(user_id);
		user.orElseThrow(() -> new UserException(UserException.USER_NOT_FOUND));
		

		user.get().setPassword(passwordEncoder.encode(password));
		
		userRepository.save(user.get());
	}

}