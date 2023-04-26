package es.ujaen.repository;

import java.util.List;
import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import es.ujaen.entity.Data;
import net.bytebuddy.asm.Advice.OffsetMapping.Sort;

public interface DataRepository extends JpaRepository<Data, Long>{
	List<Data> findByNodeId(long nodeId);
	@Query(value = "SELECT * FROM data WHERE node_id = :nodeId ORDER BY time_sample", nativeQuery = true)
	List<Data> findByNodeInfoOrderByTime(@Param("nodeId") long nodeId);
	
	@Query(value = "SELECT * FROM data WHERE node_id = :nodeId AND (time_sample BETWEEN :unixtime_from AND :unixtime_to) ORDER BY time_sample", nativeQuery = true)
	List<Data> findByNodeInfoOrderByTime(@Param("nodeId") long nodeId, @Param("unixtime_from") long unixtime_from, @Param("unixtime_to") long unixtime_to);

	@Query(value = "SELECT id,hash,node_id,output,time_sample,time_upload,:values FROM data WHERE node_id = :nodeId ORDER BY time_sample", nativeQuery = true)
	List<Object> selectPersonalizedfindByNodeIdOrderByTime(@Param("nodeId") long nodeId, @Param("values") String values);
	
	@Query(value = "SELECT count(*) FROM data WHERE node_id = :nodeId", nativeQuery = true)
	long countByNodeId(@Param("nodeId") long nodeId);
	
	Optional<Data> findByHash(String hash);

}
