if [ ! -f .env ]; then
  echo ".env doesn't exists!!!"
  exit 1
fi
export $(grep -v '^#' .env | xargs)