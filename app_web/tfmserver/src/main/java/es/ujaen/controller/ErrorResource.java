package es.ujaen.controller;

import javax.servlet.RequestDispatcher;
import javax.servlet.http.HttpServletRequest;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.web.servlet.error.ErrorController;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;

import es.ujaen.TfmserverApplication;

@Controller
public class ErrorResource implements ErrorController  {

	private static Logger logger = LoggerFactory.getLogger(ErrorResource.class);

	
    @RequestMapping("/error")
    public String handleError(HttpServletRequest request) {
    	Object status = request.getAttribute(RequestDispatcher.ERROR_STATUS_CODE);
    	
    	if (status != null) {
            Integer statusCode = Integer.valueOf(status.toString());
        
            if(statusCode == HttpStatus.NOT_FOUND.value()) {
                return "error/404";
            } else if(statusCode == HttpStatus.BAD_REQUEST.value()) {
                return "error/400";
            }else if(statusCode == HttpStatus.UNAUTHORIZED.value()) {
                return "error/401";
            } else if(statusCode == HttpStatus.INTERNAL_SERVER_ERROR.value()) {
                return "error/500";
            }  else if(statusCode == HttpStatus.FORBIDDEN.value()) {
                return "error/403";
            } else if(statusCode == HttpStatus.REQUEST_HEADER_FIELDS_TOO_LARGE.value()) {
            	logger.error("error 431");
            	return "error/error";
            }
        }
        return "error/error";
    }

    @Override
    public String getErrorPath() {
        return null;
    }
}