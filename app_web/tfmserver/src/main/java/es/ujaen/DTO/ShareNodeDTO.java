package es.ujaen.DTO;

import java.util.Date;
import java.util.List;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ShareNodeDTO {
	
	private static final Logger logger = LoggerFactory.getLogger(ShareNodeDTO.class);

	
	List<Long> nodesList;
	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9_, ]{1,512}$", message = "Error en la validación")
	String usernames;

	
	
	public ShareNodeDTO() {
		super();
	}



	public ShareNodeDTO(List<Long> nodesList,
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9,]{1,512}$", message = "Error en la validación") String usernames) {
		super();
		this.nodesList = nodesList;
		this.usernames = usernames;
	}



	public List<Long> getNodesList() {
		return nodesList;
	}



	public void setNodesList(List<Long> nodesList) {
		this.nodesList = nodesList;
	}



	public String getUsernames() {
		return usernames;
	}



	public void setUsernames(String usernames) {
		this.usernames = usernames;
	}



	@Override
	public String toString() {
		return "ShareNodeDTO [nodesList=" + nodesList + ", usernames=" + usernames + "]";
	}

	
	
}
