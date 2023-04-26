package es.ujaen.securingweb;

import java.util.ArrayList;

import javax.servlet.http.HttpServletRequest;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.session.SessionRegistry;
import org.springframework.stereotype.Component;

import java.util.List;

import org.springframework.context.ApplicationListener;
import org.springframework.security.authentication.event.AuthenticationFailureBadCredentialsEvent;
import org.springframework.security.authentication.event.InteractiveAuthenticationSuccessEvent;
import org.springframework.security.core.session.SessionInformation;
import org.springframework.security.core.session.SessionRegistry;

@Component
public class AuthenticationSuccessListener implements ApplicationListener<InteractiveAuthenticationSuccessEvent> {

//    //@Autowired
//    private SessionRegistry sessionRegistry;
    
    @Autowired
    private HttpServletRequest request;

    
    @Override
    public void onApplicationEvent(InteractiveAuthenticationSuccessEvent event) {
        // Obtiene el usuario autenticado actualmente
        Object principal = event.getAuthentication().getPrincipal();

        // Invalida todas las sesiones antiguas del usuario
//        List<SessionInformation> sessions = sessionRegistry.getAllSessions(request.getRemoteUser(), false);
//        for (SessionInformation session : sessions) {
//            session.expireNow();
//        }
//
//        // Registra la nueva sesión del usuario
//        sessionRegistry.registerNewSession(request.getSession().getId(), principal);
    }
}