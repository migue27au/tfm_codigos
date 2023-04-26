package es.ujaen.DTO;

import java.util.Date;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class AddNodeDTO {
	
	private static final Logger logger = LoggerFactory.getLogger(AddNodeDTO.class);

	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9_]{8,8}$", message = "Error en la validación")
	private String nodeName;
	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9_]{8,8}$", message = "Error en la validación")
	private String nodePass;
	
	@NotNull
	@Pattern(regexp = "^[a-zA-Z0-9]{0,25}$", message = "Error en la validación")
	private String authName;
	
	@NotNull
	@Pattern(regexp = "^[a-zA-Z0-9]{0,25}$", message = "Error en la validación")
	private String authPass;
	public AddNodeDTO() {
		super();
	}

	public AddNodeDTO(
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{1,8}$", message = "Error en la validación") String nodeName,
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{1,8}$", message = "Error en la validación") String nodePass) {
		super();
		this.nodeName = nodeName;
		this.nodePass = nodePass;
		this.nodeName = "";
		this.nodePass = "";
	}
	
	public AddNodeDTO(
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{1,8}$", message = "Error en la validación") String nodeName,
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{1,8}$", message = "Error en la validación") String nodePass,
			@NotNull @Pattern(regexp = "^[a-zA-Z0-9]{0,25}$", message = "Error en la validación") String authName,
			@NotNull @Pattern(regexp = "^[a-zA-Z0-9]{0,25}$", message = "Error en la validación") String authPass) {
		super();
		this.nodeName = nodeName;
		this.nodePass = nodePass;
		this.nodeName = authName;
		this.nodePass = authPass;
	}

	public String getNodeName() {
		return nodeName;
	}

	public void setNodeName(String nodeName) {
		this.nodeName = nodeName;
	}

	public String getNodePass() {
		return nodePass;
	}

	public void setNodePass(String nodePass) {
		this.nodePass = nodePass;
	}

	public boolean isMaster() {
		if(authName.equals("") == false && authPass.equals("") == false) {
			return true;
		} else {
			return false;
		}
	}

	public String getAuthName() {
		return authName;
	}

	public void setAuthName(String authName) {
		this.authName = authName;
	}

	public String getAuthPass() {
		return authPass;
	}

	public void setAuthPass(String authPass) {
		this.authPass = authPass;
	}		
	
	public boolean validation() {
		
		logger.info(this.toString());
		
		if(nodeName.length() >= 0 && nodeName.length() <= 8 && nodePass.length() > 0 && nodePass.length() <= 8) {
			if(authName.equals("") == true && authPass.equals("") == true) {
				return true;
			} else if(authName.equals("") == false && authPass.equals("") == false) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}

	@Override
	public String toString() {
		return "AddNodeDTO [nodeName=" + nodeName + ", nodePass=" + nodePass + ", authName=" + authName + ", authPass="
				+ authPass + "]";
	}
		
}
