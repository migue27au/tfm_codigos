package es.ujaen.utils;	

public class Location {
	
	
	
	private double latitude;
	private double longitude;
	private String timeZone;	//+08:00
	
	
	public Location(double latitude, double longitude, String timeZone) {
		super();
		this.latitude = latitude;
		this.longitude = longitude;
		this.timeZone = timeZone;
	}


	public double getLatitude() {
		return latitude;
	}


	public void setLatitude(double latitude) {
		this.latitude = latitude;
	}


	public double getLongitude() {
		return longitude;
	}


	public void setLongitude(double longitude) {
		this.longitude = longitude;
	}


	
	public String getTimeZone() {
		return timeZone;
	}
	


	public void setTimeZone(String timeZone) {
		this.timeZone = timeZone;
	}



	@Override
	public String toString() {
		return "Location [latitude=" + latitude + ", longitude=" + longitude + ", timeZone=" + timeZone + "]";
	}

	/*return the timezoneoffset in milliseconds*/
	public long getTimeZoneOffset() {
		int offset = 0;
		offset = Integer.parseInt(timeZone.split(":")[0])*3600;	//hours*3600 = seconds
		offset += Integer.parseInt(timeZone.split(":")[1])*60;	//minutes*60 = seconds
		return offset*1000;	//Milliseconds
	}	
	
}
