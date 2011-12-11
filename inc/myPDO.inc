<?php

/**
 * class myAccessPdo
 *
 * Description for class myAccessPdo
 *
 * @author:
*/
class myPDO extends PDO {

                
	private $dbInfo 	= array();
	private $enableError 	= true;	
	public 	$result; //last query result
	public 	$lastQuery;
        private $loger;

         public function __destruct() {
           if($this->loger)$this->loger->logfile_close();
        }

	public function __construct($host, $dbname, $user, $pass, $dbType='mysql'){
		try {
			
			$charset = array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES 'gbk';");//设置字符集
			if($dbType!='mysql') $charset = array();
			if($dbType=='oci'){
				putenv("ORACLE_HOME=/usr/lib/oracle/11.1/client64/lib");//本函数用来配置系统环境变量。
				parent::__construct("$dbType:dbname=//$host/$dbname;charset=ZHS16GBK", $user, $pass);
				$this->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);//设置参数
			}else{
				parent::__construct("$dbType:host=$host;dbname=$dbname", $user, $pass, $charset);
			}
			$this->dbInfo['$host'] 		= $host;
			$this->dbInfo['$dbname'] 	= $dbname;
			$this->dbInfo['$user'] 		= $user;
			$this->dbInfo['$pass'] 		= $pass;
			$this->dbInfo['$dbType'] 	= $dbType;
						
			if ($this->enableError) {//如果存在错误放入文档中
				$this->loger = new pdoLoger();
                                $this->loger->logfile_init();
				$this->displayError();				
			}
									
		} catch (PDOException $e) {		  			
                        $loger = new pdoLoger();
                        $loger->logfile_init();
                        $loger->logErr(__FUNCTION__." Error!: " . $e->getMessage() . "\n", $e);                      
                        $loger->logfile_close();                    
		}
	}
		
        
         private function bindParams($sth,$bindArrParams) {
		if(count($bindArrParams)>0) {
			$key = array_keys($bindArrParams);
			if (!is_numeric($key[0]) && (substr($key[0],0,1)==':')) {
				foreach ($bindArrParams as $keyParams => $valueParams) {
					$sth->bindValue($keyParams,$valueParams);
				}
				$this->result = $sth->execute();
			} else {

				$this->result = $sth->execute($bindArrParams);
			}
		}else {
			$this->result = $sth->execute();
		}
		return $sth;

	}
        
        //以下为新定义的方法                       
	public function exec($query,$bindArrParams=array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth,$bindArrParams);
			$sth->closeCursor();
			return resultData::set($this->result,$this->lastQuery,$bindArrParams);

		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " .$e->getMessage() . "\n", $e);
			$result= new resultData(array(),$query,$bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	//执行变更，返回修改行数
	public function update($query,$bindArrParams=array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth,$bindArrParams);
			$rowCount = $sth->rowCount();
			$sth->closeCursor();// sblocco stment
			return resultData::set($rowCount,$this->lastQuery,$bindArrParams);

		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " .$e->getMessage() . "\n", $e);
			$result= new resultData(-1,$query,$bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}
	
	public function insert($query,$bindArrParams=array()) {
		try {

			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth,$bindArrParams);
			$sth->closeCursor();
			return resultData::set($this->lastInsertId(),$this->lastQuery,$bindArrParams);

		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " .$e->getMessage() . "\n", $e);
			$result= new resultData(-1,$query,$bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}

	public function getAll($query,$bindArrParams=array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth,$bindArrParams);
			$this->result = $sth->fetchAll();
			$sth->closeCursor();
			return resultData::set($this->result,$this->lastQuery,$bindArrParams);

		} catch (PDOException $e) {

			$this->logErr(__FUNCTION__." Error!: " . $e->getMessage() . "\n", $e);
			$result= new resultData(array(),$query,$bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}
        
	public function get($query,$bindArrParams=array()) {
		try {
			$this->lastQuery = $query;
			$sth = parent::prepare($query);
			$sth = $this->bindParams($sth,$bindArrParams);
			$this->result = $sth->fetch();
			$sth->closeCursor();
			return resultData::set($this->result,$this->lastQuery,$bindArrParams);

		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " . $e->getMessage() . "\n", $e);
			$result= new resultData(array(),$query,$bindArrParams);
			$result->set_error($e);
			return $result;
		}
	}
        
        // 新定义的方法结束
    
        
	public function lastInsertId($name=''){
		try {
			$id = parent::lastInsertId($name);	
			return $id;
			
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " . $e->getMessage() . "\n", $e);
		}
	}
	
	public function closeConnection(){
		try {		
			$this->dbh = null;
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " . $e->getMessage() . "\n", $e);
		}
	}
			
	public function displayError(){
		try {
			parent::setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
		} catch (PDOException $e) {
			$this->logErr(__FUNCTION__." Error!: " . $e->getMessage() . "\n", $e);
		}
	}
		
	public function getErrorCode(){
		return parent::errorCode();
	}
		
	public function getErrorInfo(){
		return parent::errorInfo();
	}
			
	public function logErr($str,$e){
                $this->loger->logErr($str, $e);
	}	
		

}

class pdoLoger {

	private $LOGFILE = "";
	var $FILE_HANDLER;

	static function forceDirectory($dir) { // force directory structure
		return is_dir($dir) or (pdoLoger::forceDirectory(dirname($dir)) and mkdir($dir, 0777));
	}
	function logfile_init() {
		if(empty($this->LOGFILE)) {
			$filename = COMMConf::$log["log_path"].'pdo'.date('Ymd').'.log';
			$this->LOGFILE=$filename;
		}else {
			$filename = $this->LOGFILE;
		}
		pdoLoger::forceDirectory(COMMConf::$log["log_path"]);
		$this->FILE_HANDLER = fopen($filename,'a');
	}
	public function logErr($str,$e) {
		$file_size=filesize($this->LOGFILE);
		if($file_size>1000000000) return ;
		$err= "\n".date(("Ymd H:i:s"))."-->".$str.":::".print_r($e,true)."<--";
		substr($err, 0,15000);
		if ($this->FILE_HANDLER) {
			fwrite($this->FILE_HANDLER,$err);//暂时用写入log，空间不够
		}
	}

	public function logfile_close() {
		if ($this->FILE_HANDLER) {
			fclose($this->FILE_HANDLER) ;
		}

	}

}


class resultData {


	private $query = null;
	private $par = null;
	private $data = array();
	private $error = null;

	public function  __construct($data,$query,$par) {
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

	public function getQuery(){
		return $this->query;
	}

	public function count() {
		if(is_int($this->data)) return $this->data;
		return count($this->data);
	}
	public function isEmpty() {
		return empty($this->data);
	}
	public function row($index=0) {
		return $this->data[$index];
	}
	public static function set($data,$query,$par) {
		return new resultData($data,$query,$par);
	}

	public function set_error($e) {
		$this->error=$e;
	}

	public function resultId() {
		if(!is_array($this->data)) return $this->data;
	}

}

class nullPDO{

    private function setNull($query,$bindArrParams){
          $result= new resultData(array(),$query,$bindArrParams);
          $result->set_error("nullpdo");
            return $result;
    }
    
    public function exec($query,$bindArrParams=array()) {
          return $this->setNull($query,$bindArrParams);
    }
    public function update($query,$bindArrParams=array()) {
            return $this->setNull($query,$bindArrParams);
    }
    public function insert($query,$bindArrParams=array()) {
            return $this->setNull($query,$bindArrParams);
    }
    public function getAll($query,$bindArrParams=array()) {
            return $this->setNull($query,$bindArrParams);
    }
    
    public function get($query,$bindArrParams=array()) {
             return $this->setNull($query,$bindArrParams);
    }
    
    public function rowPrepare($query,$bindArrParams=array()) {
            return $this->setNull($query,$bindArrParams);
    }
    public function allPrepare($query,$bindArrParams=array()) {
          return $this->setNull($query,$bindArrParams);
    }

    public function changePrepare($query,$bindArrParams=array()) {
        return $this->setNull($query,$bindArrParams);
    }

    public function simplePrepare($query,$bindArrParams=array()) {
         return $this->setNull($query,$bindArrParams);
    }
}

class Info implements ArrayAccess {

	function offsetSet($key, $value) {
		if ( array_key_exists($key,get_object_vars($this)) ) {
			$this->{$key} = $value;
		}
	}


	function offsetGet($key) {
		if ( array_key_exists($key,get_object_vars($this)) ) {
			return $this->{$key};
		}
	}

	function offsetUnset($key) {
		if ( array_key_exists($key,get_object_vars($this)) ) {
			unset($this->{$key});
		}
	}

	function offsetExists($offset) {
		return array_key_exists($offset,get_object_vars($this));
	}
}

class DbInstrance {

   public static function getDbInstrance($data)
    {
        static $instance;
        $key=$data["server"]."_".$data["database"]."_".$data["username"];
//        if(!is_array($instance))
//        {
//            $instance=array();
//        }
        if (!isset($instance[$key])||!is_object($instance[$key]))
        {
            $instance[$key] = new myAccessPDO($data["server"],$data["database"],$data["username"],$data["password"],$data['dbtype']);
        }
        return  $instance[$key];
    }

}

?>
