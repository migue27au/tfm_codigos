package es.ujaen.securingweb;

import java.io.IOException;
import java.security.SecureRandom;
import java.util.Base64;

import javax.servlet.FilterChain;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpServletResponseWrapper;
import javax.servlet.http.HttpSession;

import org.apache.commons.lang3.StringUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.GenericFilterBean;

import es.ujaen.controller.HomeResource;

@Component
public class CSPNonceFilter extends GenericFilterBean {
	
	private static final Logger logger = LoggerFactory.getLogger(CSPNonceFilter.class);

	
    private static final int NONCE_SIZE = 32; //recommended is at least 128 bits/16 bytes
    private static final String CSP_NONCE_ATTRIBUTE = "cspNonce";
 
    private SecureRandom secureRandom = new SecureRandom();
 
    /**
     * Wrapper to fill the nonce value
     */
    public static class CSPNonceResponseWrapper extends HttpServletResponseWrapper {
        private String nonce;
 
        public CSPNonceResponseWrapper(HttpServletResponse response, String nonce) {
            super(response);
            this.nonce = nonce;
        }
 
        @Override
        public void setHeader(String name, String value) {
            if (name.equals("Content-Security-Policy") && StringUtils.isNotBlank(value)) {
                super.setHeader(name, value.replace("{nonce}", nonce));
            } else {
                super.setHeader(name, value);
            }
        }
 
        @Override
        public void addHeader(String name, String value) {
            if (name.equals("Content-Security-Policy") && StringUtils.isNotBlank(value)) {
                super.addHeader(name, value.replace("{nonce}", nonce));
            } else {
                super.addHeader(name, value);
            }
        }
    }
    
    @Override
    public void doFilter(ServletRequest req, ServletResponse res, FilterChain chain) throws IOException, ServletException {
    	HttpServletRequest request = (HttpServletRequest) req;
    	HttpServletResponse response = (HttpServletResponse) res;
        
    	HttpSession session = request.getSession();

		byte[] nonceArray = new byte[NONCE_SIZE];
		
		secureRandom.nextBytes(nonceArray);
		
		String nonce = "";
		if(session.getAttribute(CSP_NONCE_ATTRIBUTE) == null) {
			nonce = Base64.getEncoder().encodeToString(nonceArray);
			logger.info("Generando nonce CSP para la sesion actual" + nonce);
			session.setAttribute(CSP_NONCE_ATTRIBUTE, nonce);
		} else {
			nonce = (String) session.getAttribute(CSP_NONCE_ATTRIBUTE);
		}
		
		request.setAttribute(CSP_NONCE_ATTRIBUTE, nonce);
		
		chain.doFilter(request, new CSPNonceResponseWrapper(response, nonce));
    }
 
    
}