package es.ujaen.securingweb;

import java.io.IOException;
import java.util.Locale;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.core.AuthenticationException;
import org.springframework.security.web.DefaultRedirectStrategy;
import org.springframework.security.web.RedirectStrategy;
import org.springframework.security.web.WebAttributes;
import org.springframework.security.web.authentication.SimpleUrlAuthenticationFailureHandler;
import org.springframework.stereotype.Component;
import org.springframework.web.servlet.LocaleResolver;

import es.ujaen.service.LoginAttemptService;
import es.ujaen.service.UserService;

@Configuration
public class CustomLoginFailureHandler extends SimpleUrlAuthenticationFailureHandler {

	private static final Logger logger = LoggerFactory.getLogger(CustomLoginFailureHandler.class);
	
    @Autowired
    private LoginAttemptService loginAttemptService;

    @Override
    public void onAuthenticationFailure(final HttpServletRequest request, final HttpServletResponse response, final AuthenticationException exception) throws IOException, ServletException {

        super.onAuthenticationFailure(request, response, exception);        
        String failurl = "/?error=true";
        if (loginAttemptService.isBlocked()) {
        	failurl = "/?blocked=true";
        }
        super.setDefaultFailureUrl(failurl);

        request.getSession().setAttribute(WebAttributes.AUTHENTICATION_EXCEPTION, "Fail limit reached");
        
    }

}
