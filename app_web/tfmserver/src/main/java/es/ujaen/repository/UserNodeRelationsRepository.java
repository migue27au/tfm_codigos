package es.ujaen.repository;

import java.util.List;
import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;

import es.ujaen.entity.Node;
import es.ujaen.entity.UserNodeRelations;

public interface UserNodeRelationsRepository extends JpaRepository<UserNodeRelations, Long>{
	Optional<UserNodeRelations> findByAuthName(String authName);
	
	//@Query(value = "SELECT * FROM user_node_relations WHERE node_ids LIKE '%:id%'", nativeQuery = true)
	Optional<UserNodeRelations> findByNodeIdsContains(String pattern);
}
