package es.ujaen.DTO;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class NewPasswordDTO {

	private static final Logger logger = LoggerFactory.getLogger(NewPasswordDTO.class);

	String password = "";
	String passwordConfirm = "";
	public NewPasswordDTO(String password, String passwordConfirm) {
		super();
		this.password = password;
		this.passwordConfirm = passwordConfirm;
	}
	public NewPasswordDTO() {
		super();
	}
	public String getPassword() {
		return password;
	}
	public void setPassword(String password) {
		this.password = password;
	}
	public String getPasswordConfirm() {
		return passwordConfirm;
	}
	public void setPasswordConfirm(String passwordConfirm) {
		this.passwordConfirm = passwordConfirm;
	}
	
	@Override
	public String toString() {
		return "NewPasswordDTO [password=" + password + ", passwordConfirm=" + passwordConfirm + "]";
	}
	
	
}
