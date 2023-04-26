package es.ujaen.repository;

import java.util.Optional;

import org.springframework.data.jpa.repository.JpaRepository;


import es.ujaen.entity.User;

public interface UserRepository extends JpaRepository<User, Long>{
	Optional<User> findByUsername(String username);
}
