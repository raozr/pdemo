<?php

/**
 * class myAccessPdo
 *
 * Description for class myAccessPdo
 *
 * @author:
 */
class myPDO extends PDO {

	private $enableError = true;
	public $result; //last query result
	public $lastQuery;
	private $loger;

	public function __destruct() {
		if ($this->loger)
			$this->loger->logfile_close();
	}

	public function __construct($host, $dbname, $user, $pass, $dbType = 'mysql') {
		try {
			if ($dbType == 'oci') {
				$oracle_home_path = "/usr/lib/oracle/11.1/client64/lib";
				if (is_dir($oracle_home_path))
					putenv("ORACLE_HOME=" . $oracle_home_path);
				parent::__construct("$dbType:dbname=//$host/$dbname;charset=ZHS16GBK", $user, $pass);
				$this->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
			}elseif ($dbType == 'dblib') {
				parent::__construct("$dbType:host=$host;dbname=$dbname;charset=GBK", $user, $pass);
			} else {
				$charset = array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES 'gbk';");
				parent::__construct("$dbType:host=$host;dbname=$dbname", $user, $pass, $charset);
			}
			if ($this->enableError) {
				$this->loger = new pdoLoger();
				$this->loger->logfile_init();
				$this->displayError();
			}
		} catch (PDOException $e) {
			$loger = new pdoLoger();
			$loger->logfile_init();
			$loger->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
			$loger->logfile_close();
		}
	}

	private function bindParams($sth, $bindArrParams) {
		if (count($bindArrParams) > 0) {
			$key = array_keys($bindArrParams);
			if (!is_numeric($key[0]) && (substr($key[0], 0, 1) == ':')) {
				foreach ($bindArrParams as $keyParams => $valueParams) {
					$sth->bindValue($keyParams, $valueParams);
				}
				$this->result = $sth->execute();
			} else {

				$this->result = $sth->execute($bindArrParams);
			}
		} else {
			$this->result = $sth->execute();
		}
		return $sth;
	}

	public function exec($query, $bindArrParams = array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth, $bindArrParams);
			$sth->closeCursor();
			return resultData::set($this->result, $this->lastQuery, $bindArrParams);
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
			$result = new resultData(array(), $query, $bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	//执行变更，返回修改行数
	public function update($query, $bindArrParams = array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth, $bindArrParams);
			$rowCount = $sth->rowCount();
			$sth->closeCursor(); // sblocco stment
			return resultData::set($rowCount, $this->lastQuery, $bindArrParams);
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
			$result = new resultData(-1, $query, $bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	public function insert($query, $bindArrParams = array()) {
		try {

			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth, $bindArrParams);
			$sth->closeCursor();
			return resultData::set($this->lastInsertId(), $this->lastQuery, $bindArrParams);
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
			$result = new resultData(-1, $query, $bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	public function getAll($query, $bindArrParams = array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth, $bindArrParams);
			$this->result = $sth->fetchAll();
			$sth->closeCursor();
			return resultData::set($this->result, $this->lastQuery, $bindArrParams);
		} catch (PDOException $e) {

			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
			$result = new resultData(array(), $query, $bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	public function get($query, $bindArrParams = array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth, $bindArrParams);
			$this->result = $sth->fetch();
			$sth->closeCursor();
			return resultData::set($this->result, $this->lastQuery, $bindArrParams);
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
			$result = new resultData(array(), $query, $bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	public function simpleQuery($query) {
		try {
			$this->lastQuery = $query;
			$count = parent::exec($query);
			return $count;
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
		}
	}

	public function lastInsertId($name = '') {
		try {
			$id = 0;
			if (parent::getAttribute(PDO::ATTR_DRIVER_NAME) == 'dblib') {
				$sth = parent::prepare("SELECT @@IDENTITY AS ID");
				$sth->execute();
				$result = $sth->fetch();
				$sth->closeCursor();
				return $result[0];
			}


			$id = parent::lastInsertId($name);
			return $id;
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
		}
	}

	public function closeConnection() {
		try {
			$this->dbh = null;
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
		}
	}

	public function displayError() {
		try {
			parent::setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__ . " Error!: " . $e->getMessage() . "\n", $e);
		}
	}

	public function getErrorCode() {
		return parent::errorCode();
	}

	public function getErrorInfo() {
		return parent::errorInfo();
	}

	public function logErr($str, $e) {
		$this->loger->logErr($str, $e);
	}

}

class pdoLoger {

	private $LOGFILE = "";
	var $FILE_HANDLER;

	static function forceDirectory($dir) { // force directory structure
		return is_dir($dir) or (pdoLoger::forceDirectory(dirname($dir)) and mkdir($dir, 0777));
	}

	function logfile_init($filename = '') {
		if (empty($this->LOGFILE)) {
			$filename = ($filename == '') ? Conf::$log_path . 'pdo' . date('Ymd') . '.log' : $filename;
			$this->LOGFILE = $filename;
		} else {
			$filename = $this->LOGFILE;
		}
		pdoLoger::forceDirectory(Conf::$log_path);
		$this->FILE_HANDLER = fopen($filename, 'a');
	}

	public function logErr($str, $e) {
		$file_size = filesize($this->LOGFILE);
                $max_size = 104857600;   #104857600 ; #1000000000;
		if ($file_size > $max_size){
			$str = "log_file_size_more_than_1000000000";
                }
                if (strlen($str)  > $max_size){
			$str = "log_current_strlen_more_than_1000000000";
                }
		$err = "\n" . date(("Ymd H:i:s")) . "-->" . $str . ":::" . print_r($e, true) . "<--";
		substr($err, 0, 15000);
		if ($this->FILE_HANDLER) {
			fwrite($this->FILE_HANDLER, $err); //暂时用写入log，空间不够
		}
	}

	public function logfile_close() {
		if ($this->FILE_HANDLER) {
			fclose($this->FILE_HANDLER);
		}
	}

}

class resultData {

	private $query = null;
	private $par = null;
	private $data = array();
	private $error = null;

	public function __construct($data, $query, $par) {
		$this->data = $data;
		$this->query = $query;
		$this->par = $par;
	}

	function __get($data) {
		return $this->$data;
	}

	public function get() {
		return $this->data;
	}

	public function debug() {
		echo "<pre>";
		print_r($this);
		echo "</pre>";
	}

	public function getQuery() {
		return $this->query;
	}

	public function count() {
		if (is_int($this->data))
			return $this->data;
		return count($this->data);
	}

	public function isEmpty() {
		return empty($this->data);
	}

	public function isError() {
		return (!empty($this->error) || $this->data === -1);
	}

	public function isEmptyOrError() {
		return ($this->isEmpty() || $this->isError());
	}

	public function row($index = 0) {
		return $this->data[$index];
	}

	public static function set($data, $query, $par) {
		return new resultData($data, $query, $par);
	}

	public function set_error($e) {
		$this->error = $e;
	}

}

class nullPDO {

	public function get($query, $bindArrParams = array()) {
		$result = new resultData(array(), $query, $bindArrParams);
		$result->set_error("nullpdo");
		return $result;
	}

	public function getAll($query, $bindArrParams = array()) {
		$result = new resultData(array(), $query, $bindArrParams);
		$result->set_error("nullpdo");
		return $result;
	}

	public function update($query, $bindArrParams = array()) {
		$result = new resultData(array(), $query, $bindArrParams);
		$result->set_error("nullpdo");
		return $result;
	}

	public function insert($query, $bindArrParams = array()) {
		$result = new resultData(array(), $query, $bindArrParams);
		$result->set_error("nullpdo");
		return $result;
	}

}

class Info implements ArrayAccess {

	public $code = 1;
	public $errorstr = "no error";
	public $userstr = "no";
	public $data = array();

	public function toJson() {
		$str = "{ \"code\":" . $this->code . ",\"msg\":{ \"str\":\"" . $this->errorstr . "\",\"dialog\":" . json_encode(Tools::gbk2u8($this->userstr)) . "},";
		if (empty($this->data)) {
			$str.="\"data\":" . json_encode(Tools::gbk2u8($this->toArray())) . "}";
		} else {
			$str.="\"data\":" . json_encode(Tools::gbk2u8($this->data)) . "}";
		}
		return $str;
	}

	// echo Info::getErrorJson(-1,"","");
	public static function getErrorJson($code, $errstr, $usrstr = '') {
		$myInfo = new Info();
		$myInfo->errorstr = $errstr;
		$myInfo->userstr = $usrstr;
		$myInfo->code = $code;
		return $myInfo->toJson();
	}

	public static function getArrayToJson($data, $code = 1, $errstr = "no error", $usrstr = "no") {
		$myInfo = new Info();
		$myInfo->code = $code;
		$myInfo->errorstr = $errstr;
		$myInfo->userstr = $usrstr;
		$myInfo->data = $data;
		return $myInfo->toJson();
	}

	public function toArray() {
		$data = array();
		foreach ($this as $key => $value) {
			if ($key != 'code' && $key != 'errorstr' && $key != 'userstr' && $key != 'data')
				$data[$key] = $value;
		}
		return $data;
	}

	function offsetSet($key, $value) {
		if (array_key_exists($key, get_object_vars($this))) {
			$this->{$key} = $value;
		}
	}

	function offsetGet($key) {
		if (array_key_exists($key, get_object_vars($this))) {
			return $this->{$key};
		}
	}

	function offsetUnset($key) {
		if (array_key_exists($key, get_object_vars($this))) {
			unset($this->{$key});
		}
	}

	function offsetExists($offset) {
		return array_key_exists($offset, get_object_vars($this));
	}

}

 class Infos extends Info {

	private $container = array();

	public function offsetGet($offset) {
		return isset($this->container[$offset]) ? $this->container[$offset] : null;
	}

	public function offsetSet($offset, $value) {
		$data = is_object($value) ? $value->toArray() : $value;
		if (is_null($offset)) {
			$this->container[] = $data;
		} else {
			$this->container[$offset] = $data;
		}
	}

	public function toSlice($offset, $length) {
		return array_slice($this->container, $offset, $length);
	}

	public function count() {
		return count($this->container);
	}

	public function toJson($offset = 0, $length = 0) {
                $value = $this->container;
		if ($length > 0) {
			$value = array_slice($this->container, $offset, $length);
		}
		$str = "{ \"code\":" . $this->code . ",\"msg\":{ \"str\":\"" . $this->errorstr . "\",\"dialog\":\"" . Tools::gbk2u8($this->userstr) . "\"},";
		$str.="\"data\":" . json_encode(Tools::gbk2u8($value)) . ",";
                $str.="\"data1\":" . json_encode(Tools::gbk2u8($this->toArray())) . "}";
		return $str;
	}
 }
        

?>
