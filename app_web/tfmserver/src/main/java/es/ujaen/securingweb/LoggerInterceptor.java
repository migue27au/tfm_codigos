package es.ujaen.securingweb;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.servlet.HandlerInterceptor;
import org.springframework.web.servlet.ModelAndView;

public class LoggerInterceptor implements HandlerInterceptor {
	private static Logger logger = LoggerFactory.getLogger(LoggerInterceptor.class);

	@Override
	public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler){
		// TODO Auto-generated method stub
		try {
			String resource = request.getRequestURI();
			String method = request.getMethod();
			String user = request.getRemoteUser();
			String ipAddress = request.getHeader("X-Forward-For");
			String userAgent = request.getHeader("User-Agent");
			String cookies = request.getHeader("Cookie");
	
	        if(ipAddress==null){
	            ipAddress = request.getRemoteAddr();
	        }
			
			logger.info(String.format("Resource: %s; Method: %s; User: %s; Remote IP address: %s; User-Agent: %s; Cookies: %s", resource, method, user, ipAddress, userAgent, cookies));
			return true;
		} catch(Exception e) {
			
			logger.error(String.format("EXCEPTION: %s", e.getCause()));
			e.printStackTrace();
			
			return false;
		}
	}
	
//	@Override
//	public void postHandle(
//		HttpServletRequest request, HttpServletResponse response, Object handler,ModelAndView modelAndView) throws Exception {
//		int response_code = response.getStatus();
//		
//		logger.info("POST HANDLE: " + response_code);
//	}
}
