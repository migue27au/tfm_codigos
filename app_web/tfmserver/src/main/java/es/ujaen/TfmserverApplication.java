package es.ujaen;

import org.apache.catalina.Context;
import org.apache.catalina.connector.Connector;
import org.apache.coyote.http11.Http11NioProtocol;
import org.apache.tomcat.util.descriptor.web.ErrorPage;
import org.apache.tomcat.util.descriptor.web.SecurityCollection;
import org.apache.tomcat.util.descriptor.web.SecurityConstraint;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.builder.SpringApplicationBuilder;
import org.springframework.boot.system.SystemProperties;
import org.springframework.boot.web.embedded.tomcat.TomcatServletWebServerFactory;
import org.springframework.boot.web.servlet.server.ConfigurableServletWebServerFactory;
import org.springframework.boot.web.servlet.server.ServletWebServerFactory;
import org.springframework.boot.web.servlet.support.SpringBootServletInitializer;
import org.springframework.context.annotation.Bean;
import org.springframework.core.SpringVersion;
import org.springframework.data.jpa.repository.config.EnableJpaRepositories;
import org.springframework.http.HttpStatus;
import org.springframework.web.servlet.config.annotation.InterceptorRegistry;
import org.springframework.web.servlet.config.annotation.ViewResolverRegistry;
import org.springframework.web.servlet.view.InternalResourceViewResolver;
import org.springframework.web.servlet.view.JstlView;

import es.ujaen.repository.UserRepository;
import es.ujaen.securingweb.CustomHandlerInterceptor;
import es.ujaen.securingweb.LoggerInterceptor;

@SpringBootApplication
@EnableJpaRepositories(basePackageClasses = UserRepository.class)
public class TfmserverApplication extends SpringBootServletInitializer {
	
	private static Logger logger = LoggerFactory.getLogger(TfmserverApplication.class);

	
	public static void main(String[] args) {
		logger.info("Java version: " + SystemProperties.get("java.version"));
		logger.info("Spring version: " + SpringVersion.getVersion());
		SpringApplication.run(TfmserverApplication.class, args);
	}
	
	@Bean
	public LoggerInterceptor LoggerInterceptor() {
		return new LoggerInterceptor();
	}
	
//	@Bean
//    public ConfigurableServletWebServerFactory configurableServletWebServerFactory(){
//        TomcatServletWebServerFactory factory = new TomcatServletWebServerFactory();
//        factory.setProtocol("org.apache.coyote.http11.Http11NioProtocol");
//        factory.setPort(8850);
//        factory.addConnectorCustomizers( connector -> {
//            Http11NioProtocol protocol = (Http11NioProtocol) connector.getProtocolHandler();
//            protocol.setDisableUploadTimeout(false);
//            //protocol.setAcceptCount(200);
//            //protocol.setMaxConnections(200);
//            protocol.setMaxHeaderCount(20000);
//            protocol.setConnectionTimeout(20000);
//            protocol.setMaxHttpHeaderSize(209715200);
//            protocol.setMaxSavePostSize(4194304);
//        } );
//        return factory;
//    }
	
//	@Override
//	protected SpringApplicationBuilder configure(SpringApplicationBuilder builder) {
//		return builder.sources(TfmserverApplication.class);
//	}

//	@Bean
//	public ServletWebServerFactory servletContainer() {
//	    TomcatServletWebServerFactory tomcat = new TomcatServletWebServerFactory() {
//	        @Override
//	        protected void postProcessContext(Context context) {
//	            SecurityConstraint securityConstraint = new SecurityConstraint();
//	            securityConstraint.setUserConstraint("CONFIDENTIAL");
//	            SecurityCollection collection = new SecurityCollection();
//	            collection.addPattern("/*");
//	            securityConstraint.addCollection(collection);
//	            context.addConstraint(securityConstraint);
//	        }
//	    };
//	    tomcat.addAdditionalTomcatConnectors(redirectConnector());
//	    return tomcat;
//	}
//
//	private Connector redirectConnector() {
//	    Connector connector = new Connector("org.apache.coyote.http11.Http11NioProtocol");
//	    connector.setScheme("http");
//	    connector.setPort(8080);
//	    connector.setSecure(false);
//	    connector.setRedirectPort(8443);
//	    return connector;
//	}

}
