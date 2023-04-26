package es.ujaen.repository;

import java.util.List;
import java.util.Optional;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;


import es.ujaen.entity.Node;

public interface NodeRepository extends JpaRepository<Node, Long>{
	Optional<Node> findByNodeName(String nodeName);
	List<Node> findByOwnerId(long ownerId);
	
	//@Query(value = "SELECT * FROM node WHERE shared_with LIKE '%:id%'", nativeQuery = true)
	List<Node> findBySharedWithContains(String pattern);
	
}
