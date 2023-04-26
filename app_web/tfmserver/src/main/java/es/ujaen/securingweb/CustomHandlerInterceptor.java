package es.ujaen.securingweb;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.servlet.HandlerInterceptor;
import org.springframework.web.servlet.ModelAndView;

//NO SE UTILIZA

public class CustomHandlerInterceptor implements HandlerInterceptor{
	
	 Logger logger = LoggerFactory.getLogger(CustomHandlerInterceptor.class);
	 
	 @Override
	 public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler)
	   throws Exception {
	  
	  logger.info("Executing Before Handler method...");
	  return true;
	 }
	 @Override
	 public void postHandle(HttpServletRequest request, HttpServletResponse response, Object handler,
	   ModelAndView modelAndView) throws Exception {
	  logger.info("Executing After Handler method...");
	 }
	 @Override
	 public void afterCompletion(HttpServletRequest request, HttpServletResponse response, Object handler, Exception ex)
	   throws Exception {
	  logger.info("After completing request...");
	 
	 } 

}
