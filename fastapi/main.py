import os
from datetime import datetime, timedelta
from typing import List, Optional

from fastapi import FastAPI, HTTPException, Query, Depends
from pydantic import BaseModel, constr
from sqlalchemy import Column, Integer, Float, String, Boolean, DateTime, select, desc
from sqlalchemy.ext.asyncio import AsyncSession, create_async_engine
from sqlalchemy.orm import declarative_base, sessionmaker
from fastapi.middleware.cors import CORSMiddleware

# Recupera a URL do banco a partir das variáveis de ambiente
DATABASE_URL = os.getenv(
    "DATABASE_URL", "postgresql+asyncpg://seu_usuario:sua_senha@localhost:5432/seu_banco"
)

# Configuração do SQLAlchemy com suporte assíncrono
engine = create_async_engine(DATABASE_URL, echo=True)
AsyncSessionLocal = sessionmaker(engine, expire_on_commit=False, class_=AsyncSession)
Base = declarative_base()

# Modelo ORM para as medições
class MeasurementORM(Base):
    __tablename__ = "measurements"

    id = Column(Integer, primary_key=True, index=True)
    timestamp = Column(DateTime, default=datetime.utcnow, index=True)
    currentSetpoint = Column(Float, nullable=False)
    currentMeasured = Column(Float, nullable=False)
    mode = Column(String(2), nullable=False)
    active = Column(Boolean, nullable=False)
    pwm = Column(Integer, nullable=False)

# Modelo Pydantic para entrada de dados
class Measurement(BaseModel):
    currentSetpoint: float
    currentMeasured: float
    mode: constr(min_length=2, max_length=2)
    active: bool
    pwm: int

    class Config:
        orm_mode = True

# Inicialização da FastAPI
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Permite todas as origens; ajuste conforme necessário
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Dependência para obter uma sessão com o banco de dados
async def get_db():
    async with AsyncSessionLocal() as session:
        yield session

@app.on_event("startup")
async def startup():
    # Cria as tabelas, se ainda não existirem
    async with engine.begin() as conn:
        await conn.run_sync(Base.metadata.create_all)

@app.post("/data")
async def post_measurement(measurement: Measurement, db: AsyncSession = Depends(get_db)):
    """
    Recebe os dados do sistema embarcado e os armazena no banco de dados.
    """
    measurement_db = MeasurementORM(
        currentSetpoint=measurement.currentSetpoint,
        currentMeasured=measurement.currentMeasured,
        mode=measurement.mode,
        active=measurement.active,
        pwm=measurement.pwm,
        timestamp=datetime.utcnow()
    )
    db.add(measurement_db)
    await db.commit()
    await db.refresh(measurement_db)
    return {"message": "Dados recebidos com sucesso.", "id": measurement_db.id}

@app.get("/data/history", response_model=List[Measurement])
async def get_history(
    minutes: int = Query(..., gt=0, description="Número de minutos para buscar o histórico"),
    db: AsyncSession = Depends(get_db)
):
    """
    Retorna o histórico de medições dos últimos X minutos.
    """
    cutoff = datetime.utcnow() - timedelta(minutes=minutes)
    query = select(MeasurementORM).where(MeasurementORM.timestamp >= cutoff)
    result = await db.execute(query)
    measurements = result.scalars().all()
    return measurements

@app.get("/data/state")
async def get_current_state(db: AsyncSession = Depends(get_db)):
    """
    Retorna o estado atual (últimos valores de 'mode' e 'active') com base na medição mais recente.
    """
    query = select(MeasurementORM).order_by(desc(MeasurementORM.timestamp)).limit(1)
    result = await db.execute(query)
    latest = result.scalar_one_or_none()
    if latest is None:
        raise HTTPException(status_code=404, detail="Nenhum dado disponível ainda.")
    return {"mode": latest.mode, "active": latest.active}
