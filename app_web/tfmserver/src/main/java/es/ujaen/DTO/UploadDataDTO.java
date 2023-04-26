package es.ujaen.DTO;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class UploadDataDTO {

	private static final Logger logger = LoggerFactory.getLogger(UploadDataDTO.class);

	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9]{1,25}$", message = "Error in UploadData validation")
	String authName;
	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9]{1,64}$", message = "Error in UploadData validation")
	String nonce;
	
	@NotNull
	@NotEmpty
	@Pattern(regexp="^[a-zA-Z0-9]{64,64}$", message = "Error in UploadData validation")
	String hash;

	@NotNull
	@NotEmpty
	@Pattern(regexp="^[-A-Za-z0-9+/=]{3,}$", message = "Error in UploadData validation")
	String data;
		

	public UploadDataDTO() {
		super();
	}


	public UploadDataDTO(
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{1,25}$", message = "Error in UploadData validation") String authName,
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{1,64}$", message = "Error in UploadData validation") String nonce,
			@NotNull @NotEmpty @Pattern(regexp = "^[a-zA-Z0-9]{64,64}$", message = "Error in UploadData validation") String hash,
			@NotNull @NotEmpty @Pattern(regexp = "^[-A-Za-z0-9+/=]{3,}$", message = "Error in UploadData validation") String data) {
		super();
		this.authName = authName;
		this.nonce = nonce;
		this.hash = hash;
		this.data = data;
	}




	public String getAuthName() {
		return authName;
	}




	public void setAuthName(String authName) {
		this.authName = authName;
	}




	public String getNonce() {
		return nonce;
	}




	public void setNonce(String nonce) {
		this.nonce = nonce;
	}




	public String getHash() {
		return hash;
	}




	public void setHash(String hash) {
		this.hash = hash;
	}




	public String getData() {
		return data;
	}




	public void setData(String data) {
		this.data = data;
	}




	@Override
	public String toString() {
		return "UploadDataDTO [authName=" + authName + ", nonce=" + nonce + ", hash=" + hash + ", data=" + data + "]";
	}
}
