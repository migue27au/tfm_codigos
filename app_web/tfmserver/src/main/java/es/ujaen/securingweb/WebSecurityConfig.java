package es.ujaen.securingweb;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.function.Predicate;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.config.annotation.authentication.builders.AuthenticationManagerBuilder;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.builders.WebSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.config.annotation.web.configuration.WebSecurityConfigurerAdapter;
import org.springframework.security.core.session.SessionInformation;
import org.springframework.security.core.session.SessionRegistry;
import org.springframework.security.core.session.SessionRegistryImpl;
import org.springframework.security.core.userdetails.User;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.provisioning.InMemoryUserDetailsManager;
import org.springframework.security.web.authentication.session.ConcurrentSessionControlAuthenticationStrategy;
import org.springframework.security.web.firewall.StrictHttpFirewall;
import org.springframework.security.web.header.writers.StaticHeadersWriter;
import org.springframework.security.web.util.matcher.AntPathRequestMatcher;
import org.springframework.web.cors.CorsConfiguration;
import org.springframework.web.cors.CorsConfigurationSource;
import org.springframework.web.cors.UrlBasedCorsConfigurationSource;
import org.springframework.web.servlet.config.annotation.InterceptorRegistry;
import org.springframework.security.web.header.HeaderWriterFilter;

@EnableWebSecurity
public class WebSecurityConfig extends WebSecurityConfigurerAdapter {
	
	@Autowired
	private UserDetailsService userDetailsService;
	
	@Autowired
	private CustomLoginSuccessHandler successHandler;
	
	@Autowired
	private CustomLoginFailureHandler failureHandler;
	
	
	//@Autowired
	//private CustomAccessDeniedHandler accessDeniedHandler;
		
	@Override
	protected void configure(AuthenticationManagerBuilder auth) throws Exception{
		auth.userDetailsService(userDetailsService);
	}
	
	@Override
    public void configure(WebSecurity web) throws Exception {
        StrictHttpFirewall firewall = new StrictHttpFirewall();
        ArrayList<String> domains = new ArrayList<>();
        domains.add("migue27au.ddns.net");
        domains.add("migue27au2.ddns.net");
        Predicate<String> predicates = (string) -> domains.contains(string);
        
        firewall.setAllowedHostnames(predicates);
        web.httpFirewall(firewall);
    }
	
	@Override
	protected void configure(HttpSecurity http) throws Exception {
		
		//CSP - por defecto
		
//		http.headers().addHeaderWriter(new StaticHeadersWriter("X-Content-Security-Policy","")).
//			addHeaderWriter(new StaticHeadersWriter("X-WebKit-CSP","default-src 'self'"));
	
		//HTST - por defecto
//		http.headers().httpStrictTransportSecurity().includeSubDomains(true).maxAgeInSeconds(3000);

		//CSRF
		//http.cors().and().csrf().disable();	//Habilitar para otro momento
		http.headers()
	        .xssProtection();
		
		http.headers().contentSecurityPolicy("script-src 'nonce-{nonce}' 'unsafe-eval' 'self' ; object-src 'none'; base-uri 'self'");
		http.addFilterBefore(new CSPNonceFilter(), HeaderWriterFilter.class);
		
		http.cors().and().csrf().ignoringAntMatchers("/rest/**");
//		
		
		//Access denied handler
		//http.exceptionHandling().accessDeniedHandler(accessDeniedHandler);
		
//		//Permisos
//		http.authorizeRequests()
//			.antMatchers("/").permitAll()
//			.antMatchers("/home").permitAll();
//		
//		http.authorizeRequests()
//			.anyRequest().authenticated()
//			.and()
//			.formLogin().loginPage("/login")
//			.permitAll();
//		
//		//HTTPS
//		//http.requiresChannel().anyRequest().requiresSecure();
////		// we only need https on /auth
////		http.requiresChannel()
////        .antMatchers("/home").requiresSecure()
////        .anyRequest().requiresInsecure();
		
		http.authorizeRequests()
				.antMatchers("/admin").hasRole("ADMIN")	//under development
				.antMatchers("/nodes").authenticated()
				.antMatchers("/tables").authenticated()
				.antMatchers("/").permitAll()
				.antMatchers("/js/**").permitAll()
				.antMatchers("/img/**").permitAll()
				.antMatchers("/css/**").permitAll()
				.antMatchers("/rest/**").permitAll()
				.antMatchers("/webjars/**").permitAll()
				.anyRequest().authenticated()
			.and()
				.formLogin()
				.loginPage("/")
                .loginProcessingUrl("/login")
				.failureHandler(failureHandler)
				.successHandler(successHandler)
			.and()
				.logout()
				.deleteCookies("JSESSIONID")
				.deleteCookies("remember-me")
				.logoutRequestMatcher(new AntPathRequestMatcher("/logout"))
				.logoutSuccessUrl("/?logout=true")
			.and()
				.rememberMe().key("remember-me-secret").tokenValiditySeconds(60*24*7*2)	//2 weeks
			.and()
				.sessionManagement()	
				.invalidSessionUrl("/?expired=true")			
				.maximumSessions(1)
				.maxSessionsPreventsLogin(true)
				.expiredUrl("/?concurrent=true")
				.sessionRegistry(sessionRegistry());
    }
	
	
	@Bean
    public SessionRegistry sessionRegistry() {
        return new SessionRegistryImpl();
    }
	
	//Utilizar BCrypt para asegurar las contraseñas
	@Bean
	public PasswordEncoder getPasswordEncoder() {
		return new BCryptPasswordEncoder();
	}
	
	
}