package es.ujaen.securingweb;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.session.SessionInformation;
import org.springframework.security.core.session.SessionRegistry;
import org.springframework.security.web.DefaultRedirectStrategy;
import org.springframework.security.web.RedirectStrategy;
import org.springframework.security.web.authentication.SimpleUrlAuthenticationSuccessHandler;

import es.ujaen.entity.MyUserDetails;

@Configuration
public class CustomLoginSuccessHandler extends SimpleUrlAuthenticationSuccessHandler{
	
	private static final Logger logger = LoggerFactory.getLogger(CustomLoginSuccessHandler.class);
	
	@Autowired
	private SessionRegistry sessionRegistry;
	
	
	@Override
	public void onAuthenticationSuccess(HttpServletRequest request, HttpServletResponse response,
			Authentication authentication) throws IOException, ServletException {
		super.onAuthenticationSuccess(request, response, authentication);
		
		String username = authentication.getName();
		logger.info("New user logged: " + username);
		List<SessionInformation> sessions= getAllActiveSessionsByUsername(username);
		
		//disable concurrent loggins
		if(sessions.size() > 1) {
			logger.info("More than 1 session detected. Deleting previous sessions");
			SessionInformation lastSession = sessions.get(sessions.size() - 1);
	        for (SessionInformation session : sessions) {
                session.expireNow();
	        }
		}
	}
	
	@Override
	protected void handle(HttpServletRequest req, HttpServletResponse res, Authentication authentication) throws IOException{
		String targetUrl = determineTargetUrl(authentication);
//		String targetUrl = "/dashboard";
			
		if(res.isCommitted()) {
			return;
		}
		
		RedirectStrategy redirectStrategy = new DefaultRedirectStrategy();
		redirectStrategy.sendRedirect(req, res, targetUrl);
	}
	
	protected String determineTargetUrl(Authentication authentication) {
		String url = "";
		
		Collection <? extends GrantedAuthority> authorities = authentication.getAuthorities();
		List<String> roles = new ArrayList<String>();
		for(GrantedAuthority a : authorities) {
			roles.add(a.getAuthority());
		}
		
		//cambio la url dependiendo del rol
		if(roles.contains("ROLE_ADMIN")){
			url = "/nodes";
		} else if(roles.contains("ROLE_USER")) {
			url = "/nodes";
		}
		
		
		return url;
	}
	
	public List<SessionInformation> getAllActiveSessionsByUsername(String username) {
	    List<SessionInformation> activeSessions = new ArrayList<>();
	    for (Object principal : sessionRegistry.getAllPrincipals()) {
	        if (principal instanceof MyUserDetails) {
	        	MyUserDetails user = (MyUserDetails) principal;
	            if (user.getUsername().equals(username)) {
	                activeSessions.addAll(sessionRegistry.getAllSessions(principal, false));
	            }
	        }
	    }
	    return activeSessions;
	}
}
