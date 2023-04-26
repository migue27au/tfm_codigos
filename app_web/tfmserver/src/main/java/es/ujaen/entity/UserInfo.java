package es.ujaen.entity;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class UserInfo {
	
	private static final Logger logger = LoggerFactory.getLogger(UserInfo.class);

	
	private long id;
	private String username;
	
	
	public UserInfo() {
		super();
	}

	public UserInfo(long id, String username) {
		super();
		this.id = id;
		this.username = username;
	}
	
	public UserInfo(User user) {
		super();
		this.id = user.getId();
		this.username = user.getUsername();
	}

	public long getId() {
		return id;
	}

	public void setId(long id) {
		this.id = id;
	}

	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	@Override
	public String toString() {
		return "UserInfo [id=" + id + ", username=" + username + "]";
	}
	
	
}
